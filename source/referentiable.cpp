#include "history.manager.h"
#include "referentiable.h"
#include "referentiable.manager.h"
#include <time.h>
#include "os.log.h"
#include "os.log.format.h"
#include "session.h"
#include <cassert>

using namespace imajuscule;

Referentiable::Referentiable() :
Persistable()
, m_manager(NULL)
, m_guid(std::string(""))
, m_bHasSessionName(false)
{
}

Referentiable::Referentiable(ReferentiableManagerBase * manager, const std::string & guid) :
Persistable()
, m_manager(manager)
, m_guid(guid)
, m_bHasSessionName(false)
{
    assert(m_manager);
}

Referentiable::Referentiable(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName) :
Persistable()
, m_manager(manager)
, m_guid(guid)
, m_hintName(hintName)
, m_bHasSessionName(false)
{
    assert(m_manager);

    time_t result;
    result = time(NULL);

    struct tm * pTime = NULL;
#ifdef _WIN32
    struct tm time;
    pTime = &time;
    localtime_s(pTime, &result);
#else
    pTime = localtime(&result);
#endif

    FormatDate(pTime, m_dateOfCreation);
}

Referentiable::~Referentiable()
{}

ReferentiableManagerBase * Referentiable::getManager()
{
    return m_manager;
}

ReferentiableCmdBase* Referentiable::findSpecificInnerCmd(Command * c, ReferentiableManagerBase * rm, const std::string & hintName, bool bToInstantiate)
{
    assert(c);
    assert(rm);
    ReferentiableCmdBase* pRet = NULL;

    unsigned int countResults = 0;
    Commands::iterator it, end;
    c->traverseInnerCommands(it, end);
    for (; it != end; ++it)
    {
        if (ReferentiableCmdBase* rc = dynamic_cast<ReferentiableCmdBase*>(*it))
        {
            if (rc->manager() == rm)
            {
                if (rc->hintName() == hintName)
                {
                    bool bOk = false;
                    if (bToInstantiate)
                    {
                        bOk = rc->IsReadyToInstantiate();
                    }
                    else
                    {
                        bOk = rc->IsReadyToDeinstantiate();
                    }

                    if (bOk)
                    {
                        countResults++;
                        pRet = rc;

                        if (countResults > 1)
                        {
                            LG(ERR, "Referentiable::findSpecificInnerCmd(0x%x, 0x%x, %s, %s) : multiple (%d) results", c, rm, hintName.c_str(), bToInstantiate ? "true" : "false", countResults);
                            assert(0);
                        }
                    }
                }
            }
        }
    }

    return pRet;
}

Referentiable* Referentiable::instantiate(ReferentiableManagerBase * rm)
{
    return instantiate(rm, std::string(rm->defaultNameHint()));
}
Referentiable* Referentiable::instantiate(ReferentiableManagerBase * rm, const std::string & hintName)
{ 
    Referentiable * r = NULL;

    HistoryManager * h = HistoryManager::getInstance();

    if (Command * c = h->CurrentCommand())
    {
        if (h->IsUndoingOrRedoing())
        {
            //TODO retrieve an inner commmand that corresponds to this particular instantiation
            // To rely only upon order of innercommands, the order of an object's elements instantiations 
            // in the code must be the exact opposite of the object's elements deinstantiations.
            // I find it too constraining so:
            // option 1: use {rm,hintName} as key. 
            // But in some cases a key corresponds to multiple inner commands... so we might end up with a different state
            // from the original after a undo / redo (eg 2 formulas of two param<float> with same hint name could be exchanged
            // if they have the same Referentiable as direct father).
            // -> issue a user warning when this case is detected?
            // is there an option 2 ?

            if (ReferentiableCmdBase * rc = findSpecificInnerCmd(c, rm, hintName, true ))
            {
                r = rc->Instantiate();
            }
            else
            {
                LG(ERR, "Referentiable::instantiate : corresponding inner command not found");
                assert(0);
            }
        }
        else //h->IsUndoingOrRedoing()
        {
            // we are executing the current command so instantiate by adding an inner command
            std::vector<std::string> guids;
            r = rm->newReferentiable(hintName, guids);
        }
    }
    else // h->CurrentCommand()
    {
        // no current command so add one
        std::vector<std::string> guids;
        r = rm->newReferentiable(hintName, guids);
    }

    if (!r)
    {
        LG(ERR, "Referentiable::instantiate : an error occured, instantiate anyway without a command");
        assert(0);
        std::vector<std::string> guids;
        r = rm->newReferentiableInternal(hintName, guids);
    }

    return r;
}
void Referentiable::deinstantiate()
{
    bool bDone = false;

    Referentiable * r = NULL;
    ReferentiableManagerBase * rm = getManager();
    HistoryManager * h = HistoryManager::getInstance();

    if (Command * c = h->CurrentCommand())
    {
        if (h->IsUndoingOrRedoing())
        {
            // We could do nothing here, as deinstantiation would be done by the current command's code as it loops over innerCommands.
            // However I find it more "coherent" to find the accurate command now and reverse it

            // we have a current command and we are undoing or redoing it so the innercommand traversal is available
            if (ReferentiableCmdBase * rc = findSpecificInnerCmd(c, rm, hintName(), false))
            {
                rc->Deinstantiate();
                bDone = true;
            }
            else
            {
                LG(ERR, "Referentiable::Deinstantiate : corresponding inner command not found");
                assert(0);
            }

        }
        else //h->IsUndoingOrRedoing()
        {
            // we are executing the current command so deinstantiate by adding an inner command
            rm->RemoveRef(this);
            bDone = true;
        }
    }
    else // h->CurrentCommand()
    {
        // no current command so add one
        rm->RemoveRef(this);
        bDone = true;
    }

    if (!bDone)
    {
        LG(ERR, "Referentiable::Deinstantiate : an error occured, deinstantiate anyway without a command");
        assert(0);
        rm->RemoveRefInternal(this);
    }
}

const std::string & Referentiable::guid() const
{
    return m_guid;
}
const std::string & Referentiable::hintName() const
{
    return m_hintName;
}

const std::string & Referentiable::sessionName() const
{
    if (!m_bHasSessionName)
    {   
        LG(ERR, "Referentiable::sessionName : referentiable has no session name");
        assert(0);
    }
    return m_sessionName;
}

void Referentiable::setSessionName(const std::string & sn)
{
    LG(INFO, "Referentiable::setSessionName(%s)", sn.empty()?"NULL" : sn.c_str());
    m_sessionName = sn;
    m_bHasSessionName = true;
}
const std::string & Referentiable::creationDate() const
{
    return m_dateOfCreation;
}

eResult Referentiable::ReferentiablePersist::Save()
{
    eResult res = ILE_SUCCESS;

    Referentiable * r = ref();
    if (r)
    {
        WriteKeyData(KEY_NAME, r->m_hintName);
        WriteKeyData(KEY_DATE_CREA, r->m_dateOfCreation);
        WriteKeyData(KEY_GUID, r->m_guid);
    }
    else
    {
        LG(ERR, "Referentiable::ReferentiablePersist::Save: NULL ref");
        res = ILE_OBJECT_INVALID;
    }

    return res;
}

void Referentiable::ReferentiableLoad::LoadStringForKey(char key, std::string & sVal)
{
    LG(INFO, "ReferentiableLoad::LoadStringForKey(%d, %s) begin", key, (sVal.c_str() ? sVal.c_str() : "NULL"));

    switch (key)
    {
    case KEY_GUID:
        ref()->m_guid = sVal;
        break;

    case KEY_NAME:
        ref()->m_hintName = sVal;
        break;

    case KEY_DATE_CREA:
        ref()->m_dateOfCreation = sVal;
        break;

    default:
        LG(ERR, "ReferentiableLoad::LoadStringForKey(%d) : unknown (or future?) tag for this object : %d", key, key);
        break;
    }

    LG(INFO, "ReferentiableLoad::LoadStringForKey(%d, %s) end", key, (sVal.c_str() ? sVal.c_str() : "NULL"));
}
