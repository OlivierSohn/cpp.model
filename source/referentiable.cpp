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
, m_bHidden(false)
{
}

Referentiable::Referentiable(ReferentiableManagerBase * manager, const std::string & guid) :
Persistable()
, m_manager(manager)
, m_guid(guid)
, m_bHasSessionName(false)
, m_bHidden(false)
{
    assert(m_manager);
}

Referentiable::Referentiable(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName) :
Persistable()
, m_manager(manager)
, m_guid(guid)
, m_hintName(hintName)
, m_bHasSessionName(false)
, m_bHidden(false)
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
Referentiable* Referentiable::instantiate(ReferentiableManagerBase * rm)
{
    return instantiate(rm, std::string(rm->defaultNameHint()));
}
Referentiable* Referentiable::instantiate(ReferentiableManagerBase * rm, const std::string & hintName)
{
    return rm->newReferentiable(hintName);
}
void Referentiable::deinstantiate()
{
    getManager()->RemoveRef(this);
}

void Referentiable::Hide()
{
    m_bHidden = true;
}
bool Referentiable::isHidden()
{
    return m_bHidden;
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
    //LG(INFO, "Referentiable::setSessionName(%s)", sn.empty()?"NULL" : sn.c_str());
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
