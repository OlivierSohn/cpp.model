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
{
}

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
    if_A(r)
    {
        HistoryManager * h = HistoryManager::getInstance();

        if (h->isActive())
        {
            if (!ReferentiableDeleteCmdBase::ExecuteFromInnerCommand(*r))
                ReferentiableDeleteCmdBase::Execute(*r);
        }
        else
        {
            RemoveRefInternal(r);
        }
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

    HistoryManager * h = HistoryManager::getInstance();

    if (h->isActive())
    {
        if (!ReferentiableNewCmdBase::ExecuteFromInnerCommand(*this, nameHint, guids, r))
            r = ReferentiableNewCmdBase::Execute(*this, nameHint, guids);
    }
    else
    {
        r = newReferentiableInternal(nameHint, guids, false);
    }

    return r;
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

    size_t sizeGuids = guids.size();

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
T* ReferentiableManager<T>::New()
{
    ReferentiableManager<T>* rm = ReferentiableManager<T>::getInstance();
    if_A(rm)
        return static_cast<T*>(rm->newReferentiable());
    return NULL;
}

bool ReferentiableCmdBase::data::operator!=(const Command::data& other) const
{
    auto pOther = dynamic_cast<const ReferentiableCmdBase::data * >(&other);
    if_A(pOther)
    {
        if (m_manager != pOther->m_manager)
            return true;
        if (m_action != pOther->m_action)
            return true;
        if (m_hintName != pOther->m_hintName)
            return true;

        return false;
    }
    return true;
}
std::string ReferentiableCmdBase::data::getDesc() const 
{
    std::string desc;
    return desc;
}

ReferentiableCmdBase::data::data(Action a, std::string hintName, ReferentiableManagerBase * rm):
Command::data()
, m_action(a)
, m_hintName(hintName)
, m_manager(rm)
{}
auto ReferentiableCmdBase::data::instantiate(Action a, std::string hintName, ReferentiableManagerBase * rm) -> data*
{
    return new data(a, hintName, rm);
}

auto ReferentiableCmdBase::other(Action a)->Action
{
    switch (a)
    {
    case ACTION_DELETE:
        return ACTION_NEW;
        break;
    case ACTION_NEW:
        return ACTION_DELETE;
        break;
    default:
    case ACTION_UNKNOWN:
        A(0);
        return ACTION_UNKNOWN;
        break;
    }
}

ReferentiableCmdBase::ReferentiableCmdBase(ReferentiableManagerBase * manager, const std::string & nameHint, Action action) :
Command(new data(other(action), nameHint, manager), new data(action, nameHint, manager))
, m_manager(manager)
, m_hintName(nameHint)
{}
ReferentiableCmdBase::~ReferentiableCmdBase()
{}

bool ReferentiableCmdBase::doExecute(const Command::data & data)
{
    bool bDone = false;

    const ReferentiableCmdBase::data * pData = dynamic_cast<const ReferentiableCmdBase::data*>(&data);
    if_A(pData)
    {
        switch (pData->m_action)
        {
        case ACTION_DELETE:
            doDeinstantiate();
            bDone = true;
            break;
        case ACTION_NEW:
            doInstantiate();
            bDone = true;
            break;
        case ACTION_UNKNOWN:
            A(!"unknown action");
            break;
        }

    }
    return bDone;
}

void ReferentiableCmdBase::doInstantiate()
{
    std::vector<std::string> guids;
    bool bGUID = false;
    if (!m_GUID.empty())
    {
        bGUID = true;
        guids.push_back(m_GUID);
    }
    Referentiable * r = manager()->newReferentiableInternal(m_hintName, guids);

    if_A(r)
    {
        if (!bGUID)
            m_GUID = r->guid();
        else
            A(m_GUID == r->guid());

        A(m_hintName == r->hintName());
    }

    CommandResult res(true, r);
    observable().Notify(Event::RESULT, &res);
}

void ReferentiableCmdBase::doDeinstantiate()
{
    Referentiable * r = manager()->findByGuid(m_GUID);

    if_A (r)
    {
        manager()->RemoveRefInternal(r);
    }
}

ReferentiableCmdBase::CommandResult::CommandResult(bool bSuccess, Referentiable*ref):
Command::CommandResult(bSuccess)
, m_addr(ref)
{}
Referentiable * ReferentiableCmdBase::CommandResult::addr() const {
    return m_addr;
}


ReferentiableNewCmdBase::ReferentiableNewCmdBase(ReferentiableManagerBase & manager, const std::string & nameHint, const std::vector<std::string> guids) :
ReferentiableCmdBase(&manager, nameHint, ACTION_NEW)
, m_guids(guids)
{
    if(!guids.empty())
        m_GUID = guids.front();
}

ReferentiableNewCmdBase::~ReferentiableNewCmdBase()
{}

void ReferentiableNewCmdBase::Instantiate()
{
    if (getState() == NOT_EXECUTED)
        Command::Execute();
    else
        Redo();
}
void ReferentiableNewCmdBase::Deinstantiate()
{
    Undo();
}

void ReferentiableNewCmdBase::getSentenceDescription(std::string & desc)
{
    desc.append("new Ref. \"");
    desc.append(m_hintName);
    desc.append("\"");
}

bool ReferentiableNewCmdBase::ExecuteFromInnerCommand(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids, Referentiable*& oRefAddr)
{
    oRefAddr = NULL;

    Command::data * before = data::instantiate(ACTION_DELETE, nameHint, &rm);
    Command::data * after = data::instantiate(ACTION_NEW, nameHint, &rm);

    CommandResult r;
    resFunc f(RESULT_BY_REF(r));

    bool bDone = Command::ExecuteFromInnerCommand(
        typeid(ReferentiableNewCmdBase),
        *before,
        *after,
        NULL,
        &f);

    if (before)
        delete before;
    if (after)
        delete after;
    
    if (bDone)
    {
        A(r.Success());
        oRefAddr = r.addr();
    }

    return bDone;
}
Referentiable* ReferentiableNewCmdBase::Execute(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids)
{
    auto c = new ReferentiableNewCmdBase(rm, nameHint, guids);
    CommandResult r;
    auto reg = CommandResult::ListenToResult(*c, r);

    if (c->Command::Execute())
        c->observable().Remove(reg);

    return (r.Success()? r.addr() : NULL);
}

std::string ReferentiableCmdBase::guid() const
{
    return m_GUID;
}
std::string ReferentiableCmdBase::hintName() const
{
    return m_hintName;
}
ReferentiableManagerBase * ReferentiableCmdBase::manager() const
{
    if_A(m_manager)
        return m_manager;
    return NULL;
}


ReferentiableDeleteCmdBase::ReferentiableDeleteCmdBase(Referentiable & r) :
ReferentiableCmdBase(r.getManager(), r.hintName(), ACTION_DELETE)
{
    m_GUID = r.guid();
}

ReferentiableDeleteCmdBase::~ReferentiableDeleteCmdBase()
{}

void ReferentiableDeleteCmdBase::getSentenceDescription(std::string & desc)
{
    desc.append("delete Ref. \"");
    desc.append(m_hintName);
    desc.append("\"");
}

void ReferentiableDeleteCmdBase::Instantiate()
{
    Undo();
}
void ReferentiableDeleteCmdBase::Deinstantiate()
{
    if (getState() == NOT_EXECUTED)
        Command::Execute();
    else
        Redo();
}
bool ReferentiableDeleteCmdBase::ExecuteFromInnerCommand(Referentiable & r)
{
    std::string nameHint = r.hintName();
    ReferentiableManagerBase * rm = r.getManager();
    
    Command::data * before = data::instantiate(ACTION_NEW, nameHint, rm);
    Command::data * after = data::instantiate(ACTION_DELETE, nameHint, rm);

    bool bDone = Command::ExecuteFromInnerCommand(
        typeid(ReferentiableNewCmdBase),
        *before,
        *after);

    if (before)
        delete before;
    if (after)
        delete after;

    return bDone;
}
void ReferentiableDeleteCmdBase::Execute(Referentiable & r)
{
    (new ReferentiableDeleteCmdBase(r))->Command::Execute();
}

Referentiables * Referentiables::m_instance( NULL );
Referentiables::Referentiables()
{}
Referentiables::~Referentiables(){}
Referentiables * Referentiables::getInstance()
{
    if (!m_instance)
    {
        m_instance = new Referentiables();
        InitializeRefManagers();
    }
    
    return m_instance;
}
Referentiable* Referentiables::fromGUID(const std::string & guid)
{
    return getInstance()->findRefFromGUID(guid);
}

Referentiable* Referentiables::findRefFromGUID(const std::string & guid)
{
    for(auto man: m_managers)
    {
        if(Referentiable * ref = man->findByGuid(guid))
            return ref;
    }
    
    Referentiable * r(NULL);
    unsigned int index;
    std::string nameHint;
    if( Referentiable::ReadIndexForDiskGUID(guid, index, nameHint) )
    {
        if_A(index < m_managers.size())
        {
            std::vector<std::string> guids{guid};
            r = m_managers[index]->newReferentiable(nameHint, guids);
            r->Load(Storage::curDir(), guid);
        }
    }

    return r;
}
void Referentiables::regManager(ReferentiableManagerBase & m)
{
    A(m.index() == m_managers.size());
    m_managers.push_back(&m);
}
void Referentiables::registerManager(ReferentiableManagerBase & m)
{
    return getInstance()->regManager(m);
}