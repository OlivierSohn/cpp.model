#ifdef _WIN32
#define NOMINMAX
#include "Windows.h"
#include "Objbase.h"
#elif __ANDROID__
#else
#include <uuid/uuid.h>
#endif

#include <algorithm>

#include "referentiable.h"
#include "referentiable.manager.h"
#include "history.manager.h"

#include "os.log.h"
#include "os.log.format.h"

using namespace imajuscule;

ReferentiableManagerBase::ReferentiableManagerBase():
Visitable()
, m_observable(Observable<Event, Referentiable*>::instantiate())
, session_name_last_suffix(0)
{
}

ReferentiableManagerBase::~ReferentiableManagerBase()
{
    observable().Notify(Event::MANAGER_DELETE, NULL);
    observable().deinstantiate();

    // doc F3F7C744-0B78-4750-A0A1-7A9BAD872188
    // todo to fix this, manage ownership throughout the application
    for(auto ref : refs) {
        LG(ERR, "ref %s was not deinstantiated", ref->sessionName().c_str());
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
            if (likely(it == m_guidsToRftbls.end()))
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
            if (likely(it == m_snsToRftbls.end()))
            {
                m_snsToRftbls.insert(it, guidsToRftbls::value_type(sessionName, r));
            }
            else
            {
                A(!"an element was not found in guid map but found in session names map!");
            }

            refs.push_back(r);
        }
    }

    return bRet;
}

void ReferentiableManagerBase::RemoveRefInternal(Referentiable*r)
{
    if_A (r)
    {
        std::string guid = r->guid();
        std::string sessionName = r->sessionName();

        //LG(INFO, "delete %s %s", guid.c_str(), sessionName.c_str());
        
        // during destruction, the object must be accessible via its manager
        // because for example when destructing a joint, we need to launch a command to change the parent to NULL and this command uses the manager to find the object
        // that's why delete is done before removing guid and session name from maps
        r->observableReferentiable().Notify(Referentiable::Event::DEACTIVATE_LINKS, r);
        r->observableReferentiable().Notify(Referentiable::Event::WILL_BE_DELETED, r);
        delete r;

        size_t count = m_guidsToRftbls.erase(guid);
        A(count == 1);
        
        count = m_snsToRftbls.erase(sessionName);
        A(count == 1);

        for(auto it = refs.begin(); it != refs.end(); ++it) {
            if(*it == r) {
                refs.erase(it);
                break;
            }
        }

        observable().Notify(Event::RFTBL_REMOVE, r); // must be placed after actual delete (use case : delete of joint makes the parent NULL so the joint ui manager draws a joint at root which must be removed)
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

referentiables ReferentiableManagerBase::ListReferentiablesByCreationDate() const
{
    referentiables vItems = refs;

    std::sort(vItems.begin(), vItems.end(), pred());
    
    return vItems;
}

Referentiable * ReferentiableManagerBase::findByGuid(const std::string & guid)
{
    //LG(INFO, "ReferentiableManagerBase::findByGuid(%s)", guid.c_str());
    guidsToRftbls::iterator it = m_guidsToRftbls.find(guidsToRftbls::key_type(guid));
    if (it != m_guidsToRftbls.end()) {
        return it->second;
    }
    return NULL;
}
// session name is unique per-session
Referentiable * ReferentiableManagerBase::findBySessionName(const std::string & sessionName)
{
    snsToRftbls::iterator it = m_snsToRftbls.find(snsToRftbls::key_type(sessionName));
    if (it != m_snsToRftbls.end()) {
        return it->second;
    }
    return NULL;
}

bool ReferentiableManagerBase::ComputeSessionName(Referentiable * r, bool bFinalize)
{
    bool bRet = false;

    if_A (r)
    {
        std::string sessionName = r->hintName();

        std::transform(sessionName.begin(), sessionName.end(), sessionName.begin(), ::toupper);

        if(findBySessionName(sessionName))
        {
            sessionName += "_";
         
            auto f = [this](const std::string & pre, int i) -> bool {
                return findBySessionName(pre + std::to_string(i)) ? true: false;
            };
            
            while (f(sessionName,session_name_last_suffix))
            {
                session_name_last_suffix++;
            }
            
            sessionName += std::to_string(session_name_last_suffix);
            session_name_last_suffix++;
        }
        bRet = RegisterWithSessionName(r, sessionName);
    }

    if(bRet && bFinalize)
    {
        r->Init();
        r->onLoaded();
        
        observable().Notify(ReferentiableManagerBase::Event::RFTBL_ADD, r);
    }
    
    return bRet;
}

std::string ReferentiableManagerBase::generateGuid()
{
    //LG(INFO, "ReferentiableManagerBase::generateGuid : begin");

    std::string sGuid;
    sGuid.reserve(32);

#ifdef _WIN32
    GUID guid;
	HRESULT hr = CoCreateGuid(&guid);
	if (unlikely(FAILED(hr)))
	{
		LG(ERR, "ReferentiableManagerBase::generateGuid : CoCreateGuid failed %x", hr);
		A(0);
		return sGuid;
	}

    OLECHAR* bstrGuid;
    hr = StringFromCLSID(guid, &bstrGuid);
	if (unlikely(FAILED(hr)))
	{
		LG(ERR, "ReferentiableManagerBase::generateGuid : StringFromCLSID failed %x", hr);
		A(0);
		return sGuid;
	}

    // First figure out our required buffer size.
    int cbData = WideCharToMultiByte(CP_ACP, 0, bstrGuid/*pszDataIn*/, -1, NULL, 0, NULL, NULL);
    hr = (cbData == 0) ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
    if (likely(SUCCEEDED(hr)))
    {
        // Now allocate a buffer of the required size, and call WideCharToMultiByte again to do the actual conversion.
        std::unique_ptr<char[]> pszData( new (std::nothrow) CHAR[cbData] );
        hr = pszData.get() ? S_OK : E_OUTOFMEMORY;
        if (likely(SUCCEEDED(hr)))
        {
            hr = WideCharToMultiByte(CP_ACP, 0, bstrGuid/*pszDataIn*/, -1, pszData.get(), cbData, NULL, NULL)
                ? S_OK
                : HRESULT_FROM_WIN32(GetLastError());
            if (likely(SUCCEEDED(hr)))
            {
                for( int i=0; i<cbData; i++ ) {
                    switch ( pszData[i] ) {
                    default:
                        sGuid.push_back(pszData[i]);
                        break;
                    case 0:
                    case '-':
                    case '}':
                    case '{':
                        break;
                    }
                }
            }
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
    for( int i=0; i<(int)sizeof(uuid); i++ ) {
        switch(uuid[i]) {
            case 0:
                break;
            case '-':
                continue;
            default:
                sGuid.push_back(uuid[i]);
                break;
        }
    }
    //sGuid.push_back('{');
    //sGuid.insert(sGuid.end(),uuid, uuid+sizeof(uuid)-1/*before end of string*/);
    //sGuid.push_back('}');
#endif

    //LG(INFO, "ReferentiableManagerBase::generateGuid returns %s", sGuid.c_str());
    
    A(sGuid.size() == 32);
    return sGuid;
}


void ReferentiableManagerBase::RemoveRef(Referentiable*r)
{
    A(r);

    HistoryManager * h = HistoryManager::getInstance();
    
    if (h->isActive())
    {
        if (!ReferentiableDeleteCmdBase::ExecuteFromInnerCommand(*r)) {
            ReferentiableDeleteCmdBase::Execute(*r);
        }
    }
    else
    {
        RemoveRefInternal(r);
    }    
}

Referentiable* ReferentiableManagerBase::newReferentiable(bool bFinalize)
{
    return newReferentiable(defaultNameHint(), bFinalize);
}

Referentiable* ReferentiableManagerBase::newReferentiable(const std::string & nameHint, bool bFinalize)
{
    return newReferentiable(nameHint, std::vector < std::string>(), bFinalize, false);
}
Referentiable* ReferentiableManagerBase::newReferentiable(const std::string & nameHint, const std::vector<std::string> & guids, bool bFinalize, bool bVisibleIfAhistoric)
{
    Referentiable * r = NULL;

    HistoryManager * h = HistoryManager::getInstance();

    if (h->isActive())
    {
        A(bFinalize);
        if (!ReferentiableNewCmdBase::ExecuteFromInnerCommand(*this, nameHint, guids, r))
            r = ReferentiableNewCmdBase::Execute(*this, nameHint, guids);
    }
    else
    {
        r = newReferentiableInternal(nameHint, guids, bVisibleIfAhistoric, bFinalize);
    }

    return r;
}
template <class T>
ReferentiableManager<T> * ReferentiableManager<T>::g_pRefManager = NULL;

template <class T>
ReferentiableManager<T> * ReferentiableManager<T>::getInstance()
{
    if (unlikely(!g_pRefManager))
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
    A(g_pRefManager == this);
    g_pRefManager = 0;
}


template <class T>
Referentiable* ReferentiableManager<T>::newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids, bool bVisible, bool bFinalize)
{
    /*LG(INFO, "ReferentiableManager<T>::newReferentiable(%s, %d guids) begin",
        (nameHint.c_str() ? nameHint.c_str() : "NULL"),
        guids.size());*/

    std::string guid;

    if (!guids.empty())
    {
        guid.assign(guids[0]);
    }
    else
    {
        guid = generateGuid();
    }

    auto ref = new T(this, guid, nameHint);
    if (!bVisible)
        ref->Hide();
    if (unlikely(!ComputeSessionName(ref, bFinalize)))
    {
        LG(ERR, "ReferentiableManager<T>::newReferentiable : ComputeSessionName failed (uuid: %s)", guid.c_str());
        delete ref;
        return 0;
    }

    //LG((ref ? INFO : ERR), "ReferentiableManager<T>::newReferentiable(...) returns 0x%x", ref);
    return ref;
}

template <class T>
ref_unique_ptr<T> ReferentiableManager<T>::New()
{
    ReferentiableManager<T>* rm = ReferentiableManager<T>::getInstance();
    A(rm);
    return ref_unique_ptr<T>( static_cast<T*>(rm->newReferentiable(true)) );
}

bool ReferentiableCmdBase::data::operator!=(const Undoable::data& other) const
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
Undoable::data()
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

bool ReferentiableCmdBase::doExecute(const Undoable::data & data)
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
Undoable::CommandResult(bSuccess)
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

void ReferentiableNewCmdBase::getSentenceDescription(std::string & desc) const
{
    desc.append("new Ref. \"");
    desc.append(m_hintName);
    desc.append("\"");
}

bool ReferentiableNewCmdBase::ExecuteFromInnerCommand(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids, Referentiable*& oRefAddr)
{
    oRefAddr = NULL;

    std::unique_ptr<Undoable::data> before(data::instantiate(ACTION_DELETE, nameHint, &rm));
    std::unique_ptr<Undoable::data> after(data::instantiate(ACTION_NEW, nameHint, &rm));

    CommandResult r;
    resFunc f(RESULT_BY_REF(r));

    bool bDone = Undoable::ExecuteFromInnerCommand<ReferentiableCmdBase>(
        *before,
        *after,
        NULL,
        &f);
    
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
    A(m_manager);
    return m_manager;
}


ReferentiableDeleteCmdBase::ReferentiableDeleteCmdBase(Referentiable & r) :
ReferentiableCmdBase(r.getManager(), r.hintName(), ACTION_DELETE)
{
    m_GUID = r.guid();
}

ReferentiableDeleteCmdBase::~ReferentiableDeleteCmdBase()
{}

void ReferentiableDeleteCmdBase::getSentenceDescription(std::string & desc) const
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
    
    std::unique_ptr<Undoable::data> before(data::instantiate(ACTION_NEW, nameHint, rm));
    std::unique_ptr<Undoable::data> after(data::instantiate(ACTION_DELETE, nameHint, rm));

    bool bDone = Undoable::ExecuteFromInnerCommand<ReferentiableCmdBase>(
        *before,
        *after);

    return bDone;
}
void ReferentiableDeleteCmdBase::Execute(Referentiable & r)
{
    (new ReferentiableDeleteCmdBase(r))->Command::Execute();
}

