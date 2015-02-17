#ifdef _WIN32
#include "Windows.h"
#include "Objbase.h"
#elif __ANDROID__
#else
#include <uuid/uuid.h>
#endif

#include "referentiable.h"
#include "referentiable.manager.h"
#include "history.manager.h"
#include "os.log.h"
#include <algorithm>
#include "os.log.format.h"

using namespace imajuscule;

ReferentiableManagerBase::ReferentiableManagerBase():
Visitable()
, m_observable(Observable<Event, Referentiable*>::instantiate())
{}

ReferentiableManagerBase::~ReferentiableManagerBase()
{
    observable().Notify(Event::MANAGER_DELETE, NULL);
    observable().deinstantiate();

    {
        guidsToRftbls::iterator it = m_guidsToRftbls.begin();
        for (; it != m_guidsToRftbls.end(); ++it)
        {
            delete (it->second);
        }
    }
}

Observable<ReferentiableManagerBase::Event, Referentiable*> & ReferentiableManagerBase::observable()
{
    return *m_observable;
}

bool ReferentiableManagerBase::RegisterWithSessionName(Referentiable * r, const std::string & sessionName)
{
    bool bRet = false;
    if_A (r)
    {
        r->setSessionName(sessionName);

        {
            std::string guid = r->guid();
            guidsToRftbls::iterator it = m_guidsToRftbls.find(guid);
            if (it == m_guidsToRftbls.end())
            {
                m_guidsToRftbls.insert(it, guidsToRftbls::value_type(guid, r));
                bRet = true;
            }
            else
            {
                A(!"guid already present");
            }
        }

        if ( bRet )
        {
            snsToRftbls::iterator it = m_snsToRftbls.find(sessionName);
            if (it == m_snsToRftbls.end())
            {
                m_snsToRftbls.insert(it, guidsToRftbls::value_type(sessionName, r));
            }
            else
            {
                A(!"an element was not found in guid map but found in session names map!");
            }

            r->Init();

            observable().Notify(Event::RFTBL_ADD, r);
        }
    }

    return bRet;
}

const char * ReferentiableManagerBase::defaultNameHint()
{
    return "Referentiable";
}

void ReferentiableManagerBase::RemoveRefInternal(Referentiable*r)
{
    if_A (r)
    {
        std::string guid = r->guid();
        std::string sessionName = r->sessionName();

        // during destruction, the object must be accessible via its manager 
        // because for example when destructing a joint, we need to launch a command to change the parent to NULL and this command uses the manager to find the object
        // that's why delete is done before removing guid and session name from maps
        r->observableReferentiable().Notify(Referentiable::Event::WILL_BE_DELETED, r);
        delete r;

        size_t count = m_guidsToRftbls.erase(guid);
        A(count == 1);
        
        count = m_snsToRftbls.erase(sessionName);
        A(count == 1);

        observable().Notify(Event::RFTBL_REMOVE, r); // must be placed after actual delete (use case : delete of joint makes the parent NULL so the joint ui manager draws a joint at root which must be removed)
    }
}

void FormatDateForComparison(std::string & date)
{
    const char * numbers = "0123456789";

    if (11 < date.size())
    {
        if (2 == date.find_first_not_of(numbers, 0))
        {
            if (5 == date.find_first_not_of(numbers, 3))
            {
                if (10 == date.find_first_not_of(numbers, 6))
                {
                    //date is with format "dd?mm?yyyy?....." 
                    std::string newDate;
                    newDate.append(date.substr(6, 4));
                    newDate.append("/");
                    newDate.append(date.substr(3, 2));
                    newDate.append("/");
                    newDate.append(date.substr(0, 2));

                    newDate.append(date.substr(10));

                    A(newDate.size() == date.size());

                    date.swap(newDate);
                }
            }
        }
    }
}

struct pred
{
    bool operator()(Referentiable * const & a, Referentiable * const & b) const
    {
        A(a && b);
        std::string date1 = a->creationDate();
        std::string date2 = b->creationDate();
        FormatDateForComparison(date1);
        FormatDateForComparison(date2);

        return date1 < date2;
    }
};

void ReferentiableManagerBase::ListReferentiablesByCreationDate(referentiables& vItems)
{
    vItems.clear();

    guidsToRftbls::iterator it = m_guidsToRftbls.begin();
    guidsToRftbls::iterator end = m_guidsToRftbls.end();
    for (; it != end; ++it)
    {
        vItems.emplace_back(it->second);
    }

    std::sort(vItems.begin(), vItems.end(), pred());
}

Referentiable * ReferentiableManagerBase::findByGuid(const std::string & guid)
{
    Referentiable * pRet = NULL;
    guidsToRftbls::iterator it = m_guidsToRftbls.find(guidsToRftbls::key_type(guid));
    if (it != m_guidsToRftbls.end())
        pRet = it->second;
    return pRet;
}
// session name is unique per-session
Referentiable * ReferentiableManagerBase::findBySessionName(const std::string & sessionName)
{
    Referentiable * pRet = NULL;
    snsToRftbls::iterator it = m_snsToRftbls.find(snsToRftbls::key_type(sessionName));
    if (it != m_snsToRftbls.end())
        pRet = it->second;
    return pRet;
}

bool ReferentiableManagerBase::ComputeSessionName(Referentiable * r)
{
    bool bRet = false;

    if_A (r)
    {
        std::string sessionName = r->hintName();

        for (auto& c : sessionName) c = tolower(c);

        while (Referentiable * r2 = findBySessionName(sessionName))
        {
            sessionName.append("1");
        }
        bRet = RegisterWithSessionName(r, sessionName);
    }

    return bRet;
}

void ReferentiableManagerBase::generateGuid(std::string & sGuid)
{
    //LG(INFO, "ReferentiableManagerBase::generateGuid : begin");

    sGuid.clear();
#ifdef _WIN32
    GUID guid;
    CoCreateGuid(&guid);

    OLECHAR* bstrGuid;
    StringFromCLSID(guid, &bstrGuid);

    // First figure out our required buffer size.
    DWORD cbData = WideCharToMultiByte(CP_ACP, 0, bstrGuid/*pszDataIn*/, -1, NULL, 0, NULL, NULL);
    HRESULT hr = (cbData == 0) ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
    if (SUCCEEDED(hr))
    {
        // Now allocate a buffer of the required size, and call WideCharToMultiByte again to do the actual conversion.
        char *pszData = new (std::nothrow) CHAR[cbData];
        hr = pszData ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            hr = WideCharToMultiByte(CP_ACP, 0, bstrGuid/*pszDataIn*/, -1, pszData, cbData, NULL, NULL)
                ? S_OK
                : HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))
            {
                sGuid.clear();
                sGuid.append(pszData);
            }
            delete[] pszData;
        }
    }

    // ensure memory is freed
    ::CoTaskMemFree(bstrGuid);
#elif __ANDROID__
    LG(ERR, "ReferentiableManagerBase::generateGuid : on android, the guid should be generated in java");
#else
    uuid_t uu;
    uuid_generate(uu);
    char uuid[37];
    uuid_unparse(uu, uuid);
    sGuid.assign(uuid);
#endif

    //LG(INFO, "ReferentiableManagerBase::generateGuid returns %s", sGuid.c_str());
}


void ReferentiableManagerBase::RemoveRef(Referentiable*r)
{
    HistoryManager * h = HistoryManager::getInstance();

    if (h->isActive())
    {
        bool bDone = false;

        if (Command * c = h->CurrentCommand())
        {
            if (h->IsUndoingOrRedoing())
            {
                if (ReferentiableCmdBase * rc = findSpecificInnerCmd(c, r->hintName(), false))
                {
                    rc->Deinstantiate();
                    bDone = true;
                }
                else
                {
                    A(!"corresponding inner command not found");
                }
            }
        }

        if (!bDone)
        {
            CmdDelete(r->guid())->Execute();
        }
    }
    else
    {
        RemoveRefInternal(r);
    }
}

Referentiable* ReferentiableManagerBase::newReferentiable()
{
    return newReferentiable(defaultNameHint());
}

Referentiable* ReferentiableManagerBase::newReferentiable(const std::string & nameHint)
{
    return newReferentiable(nameHint, std::vector < std::string>());
}
Referentiable* ReferentiableManagerBase::newReferentiable(const std::string & nameHint, const std::vector<std::string> & guids)
{
    Referentiable * r = NULL;

    if (HistoryManager::getInstance()->isActive())
    {
        r = newReferentiableFromInnerCommand(nameHint, guids);

        if (!r)
        {
            ReferentiableNewCmdBase * c = CmdNew(nameHint, guids);

            if (c->Execute())
                r = c->refAddr();
        }
    }
    else
    {
        r = newReferentiableInternal(nameHint, guids, false);
    }

    return r;
}

Referentiable* ReferentiableManagerBase::newReferentiableFromInnerCommand(const std::string & nameHint, const std::vector<std::string> & guids)
{
    Referentiable * r = NULL;

    HistoryManager * h = HistoryManager::getInstance();

    if (Command * c = h->CurrentCommand())
    {
        if (h->IsUndoingOrRedoing())
        {
            // retrieve an inner commmand that corresponds to this particular instantiation, using {this,hintName} as key. 
            // In some cases a key can correspond to multiple inner commands so we might end up with a different state
            // after a undo + redo cycle, e.g. 2 formulas of two param<float> with same hint name could be exchanged
            // if they have the same Referentiable as direct father.
            // TODO? issue a user warning when this case is detected? is there another better mechanism?

            if (ReferentiableCmdBase * rc = findSpecificInnerCmd(c, nameHint, true))
            {
                r = rc->Instantiate();
            }
            else
            {
                A("corresponding inner command not found");
            }
        }
    }

    return r;
}
template <class T>
ReferentiableNewCmdBase * ReferentiableManager<T>::CmdNew(const std::string & nameHint, const std::vector<std::string> & guids)
{
    return new ReferentiableNewCmd<T>(nameHint, guids);
}
template <class T>
ReferentiableDeleteCmdBase * ReferentiableManager<T>::CmdDelete(const std::string & guid)
{
    return new ReferentiableDeleteCmd<T>(guid);
}
template <class T>
ReferentiableManager<T> * ReferentiableManager<T>::g_pRefManager = NULL;

template <class T>
ReferentiableManager<T> * ReferentiableManager<T>::getInstance()
{
    if (!g_pRefManager)
    {
        g_pRefManager = new ReferentiableManager<T>();
    }

    return g_pRefManager;
}

template <class T>
ReferentiableManager<T>::ReferentiableManager() :
ReferentiableManagerBase()
{
}

template <class T>
ReferentiableManager<T>::~ReferentiableManager()
{
}

template <class T>
Referentiable* ReferentiableManager<T>::newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids, bool bVisible)
{
    /*LG(INFO, "ReferentiableManager<T>::newReferentiable(%s, %d guids) begin",
        (nameHint.c_str() ? nameHint.c_str() : "NULL"),
        guids.size());*/

    T * ref = NULL;

    std::string guid;

    int sizeGuids = guids.size();

    if (sizeGuids > 0)
    {
        guid.assign(guids[0]);
    }
    else
    {
        generateGuid(guid);
    }

    ref = new T(this, guid, nameHint);
    if (!bVisible)
        ref->Hide();
    if (!ComputeSessionName(ref))
    {
        LG(ERR, "ReferentiableManager<T>::newReferentiable : ComputeSessionName failed (uuid: %s)", guid.c_str());
        delete ref;
        ref = NULL;
        goto end;
    }

end:

    //LG((ref ? INFO : ERR), "ReferentiableManager<T>::newReferentiable(...) returns 0x%x", ref);
    return ref;
}

template <class T>
ReferentiableNewCmd<T>::ReferentiableNewCmd(const std::string & nameHint, const std::vector<std::string> guids) :
ReferentiableNewCmdBase(nameHint, guids)
, m_manager(ReferentiableManager<T>::getInstance())
{
}
template <class T>
ReferentiableNewCmd<T>::~ReferentiableNewCmd()
{}

template <class T>
ReferentiableManagerBase * ReferentiableNewCmd<T>::manager()
{
    return m_manager;
}
template <class T>
ReferentiableDeleteCmd<T>::ReferentiableDeleteCmd(const std::string & guid) :
ReferentiableDeleteCmdBase(guid)
, m_manager(ReferentiableManager<T>::getInstance())
{
}
template <class T>
ReferentiableDeleteCmd<T>::~ReferentiableDeleteCmd()
{}

template <class T>
ReferentiableManagerBase * ReferentiableDeleteCmd<T>::manager()
{
    return m_manager;
}

ReferentiableCmdBase::ReferentiableCmdBase() :
Command()
, m_addr(NULL)
, m_after({ std::string(), std::string() })
{}
ReferentiableCmdBase::~ReferentiableCmdBase()
{}


void ReferentiableCmdBase::doInstantiate()
{
    std::vector<std::string> guids{ m_after.m_GUID };
    Referentiable * r = manager()->newReferentiableInternal(m_after.m_hintName, guids);
    m_addr = r;
    if_A(r)
    {
        A(m_after.m_GUID == r->guid());
        A(m_after.m_hintName == r->hintName());
    }
}

void ReferentiableCmdBase::doDeinstantiate()
{
    Referentiable * r = manager()->findByGuid(m_after.m_GUID);

    if_A (r)
    {
        manager()->RemoveRefInternal(r);
    }

    m_addr = NULL;
}

ReferentiableNewCmdBase::ReferentiableNewCmdBase(const std::string & nameHint, const std::vector<std::string> guids) :
ReferentiableCmdBase()
, m_params({ nameHint, guids })
{
}

ReferentiableNewCmdBase::~ReferentiableNewCmdBase()
{}

bool ReferentiableNewCmdBase::IsReadyToInstantiate() const
{
    return validStateToRedo();
}
bool ReferentiableNewCmdBase::IsReadyToDeinstantiate() const
{
    return validStateToUndo();
}
Referentiable * ReferentiableNewCmdBase::Instantiate()
{
    Redo();
    return refAddr();
}
void ReferentiableNewCmdBase::Deinstantiate()
{
    Undo();
}

void ReferentiableNewCmdBase::getDescription(std::string & desc)
{
    switch (getState())
    {
    case NOT_EXECUTED:
        desc.append("Waiting for command execution");
        break;
    default:
        desc.append("new Ref. \"");
        desc.append(m_after.m_hintName);
        desc.append("\"");
        break;
    }
}

Referentiable * ReferentiableCmdBase::refAddr() const
{
    return m_addr;
}

std::string ReferentiableCmdBase::guid() const
{
    return m_after.m_GUID;
}
std::string ReferentiableCmdBase::hintName() const
{
    return m_after.m_hintName;
}


bool ReferentiableNewCmdBase::doExecute()
{
    Referentiable * r = NULL;

    r = manager()->newReferentiableInternal(m_params.m_nameHint, m_params.m_guids);

    m_after.m_GUID = r->guid();
    m_after.m_hintName = r->hintName();
    
    A(m_params.m_nameHint == m_after.m_hintName);
    m_addr = r;

    return true;
}

void ReferentiableNewCmdBase::doUndo()
{
    doDeinstantiate();
}

void ReferentiableNewCmdBase::doRedo()
{
    doInstantiate();
}

ReferentiableDeleteCmdBase::ReferentiableDeleteCmdBase(const std::string & guid) :
ReferentiableCmdBase()
, m_bHasParameters(true)
, m_params({ guid })
{
}

ReferentiableDeleteCmdBase::~ReferentiableDeleteCmdBase()
{}

void ReferentiableDeleteCmdBase::getDescription(std::string & desc)
{
    switch (getState())
    {
    case NOT_EXECUTED:
        desc.append("Waiting for command execution");
        break;
    default:
        desc.append("delete Ref. \"");
        desc.append(m_after.m_hintName);
        desc.append("\"");
        break;
    }
}

bool ReferentiableDeleteCmdBase::IsReadyToInstantiate() const
{
    return validStateToUndo();
}
bool ReferentiableDeleteCmdBase::IsReadyToDeinstantiate() const
{
    return validStateToRedo();
}

Referentiable * ReferentiableDeleteCmdBase::Instantiate()
{
    Undo();
    return refAddr();
}
void ReferentiableDeleteCmdBase::Deinstantiate()
{
    Redo();
}

bool ReferentiableDeleteCmdBase::doExecute()
{
    Referentiable * r = NULL;

    A(m_bHasParameters);

    r = manager()->findByGuid(m_params.m_guid);

    m_after.m_GUID = r->guid();
    m_after.m_hintName = r->hintName();

    manager()->RemoveRefInternal(r);

    m_addr = NULL;

    return true;
}

void ReferentiableDeleteCmdBase::doUndo()
{
    doInstantiate();
}
void ReferentiableDeleteCmdBase::doRedo()
{
    doDeinstantiate();
}

ReferentiableCmdBase* ReferentiableManagerBase::findSpecificInnerCmd(Command * c, const std::string & hintName, bool bToInstantiate)
{
    A(c);
    ReferentiableCmdBase* pRet = NULL;

    unsigned int countResults = 0;
    Commands::iterator it, end;
    c->traverseInnerCommands(it, end);
    for (; it != end; ++it)
    {
        if (ReferentiableCmdBase* rc = dynamic_cast<ReferentiableCmdBase*>(*it))
        {
            if (rc->manager() == this)
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
                            LG(ERR, "ReferentiableManagerBase(0x%x)::findSpecificInnerCmd(0x%x, %s, %s) : multiple (%d) results", this, c, hintName.c_str(), bToInstantiate ? "true" : "false", countResults);
                            A(0);
                        }
                    }
                }
            }
        }
    }

    return pRet;
}

