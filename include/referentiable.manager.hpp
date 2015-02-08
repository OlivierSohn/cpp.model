#ifdef _WIN32
#include "Windows.h"
#include "Objbase.h"
#elif __ANDROID__
#else
#include <uuid/uuid.h>
#endif

#include "referentiable.h"
#include "referentiable.manager.h"
#include "os.log.h"
#include <cassert>
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
    if (r)
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
                LG(ERR, "ReferentiableManagerBase::Register : guid already present");
                assert(0);
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
                LG(ERR, "ReferentiableManagerBase::Register : an element was not found in guid map but found in session names map!");
                assert(0);
            }

            r->Init();

            observable().Notify(Event::RFTBL_ADD, r);
        }
    }
    else
    {
        LG(ERR, "ReferentiableManagerBase::Register : NULL param");
        assert(0);
    }

    return bRet;
}

const char * ReferentiableManagerBase::defaultNameHint()
{
    return "Referentiable";
}

void ReferentiableManagerBase::RemoveRefInternal(Referentiable*r)
{
    if (r)
    {
        size_t count = m_guidsToRftbls.erase(r->guid());
        assert(count == 1);
        
        count = m_snsToRftbls.erase(r->sessionName());
        assert(count == 1);
        
        observable().Notify(Event::RFTBL_REMOVE, r);
        delete r;
    }
    else
    {
        LG(ERR, "ReferentiableManagerBase::Remove : NULL param");
        assert(0);
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

                    assert(newDate.size() == date.size());

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
        assert(a && b);
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

    if (r)
    {
        std::string sessionName = r->hintName();

        for (auto& c : sessionName) c = tolower(c);

        while (Referentiable * r2 = findBySessionName(sessionName))
        {
            sessionName.append("1");
        }
        bRet = RegisterWithSessionName(r, sessionName);
    }
    else
    {
        LG(ERR, "ReferentiableManagerBase::ComputeSessionName: r is NULL");
        assert(0);
    }

    return bRet;
}

void ReferentiableManagerBase::generateGuid(std::string & sGuid)
{
    LG(INFO, "ReferentiableManagerBase::generateGuid : begin");

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

    LG(INFO, "ReferentiableManagerBase::generateGuid returns %s", sGuid.c_str());
}


void ReferentiableManagerBase::RemoveRef(Referentiable*r)
{
    ReferentiableDeleteCmdBase * c = CmdDelete(r->guid());

    c->Execute();
}

Referentiable* ReferentiableManagerBase::newReferentiable()
{
    ReferentiableNewCmdBase * c = CmdNew();

    if (c->Execute())
        return c->refAddr();
    else
        return NULL;
}

Referentiable* ReferentiableManagerBase::newReferentiable(const std::string & nameHint, const std::vector<std::string> & guids)
{
    ReferentiableNewCmdBase * c = CmdNew(nameHint, guids);

    if (c->Execute())
        return c->refAddr();
    else
        return NULL;
}

template <class T>
ReferentiableNewCmdBase * ReferentiableManager<T>::CmdNew()
{
    return new ReferentiableNewCmd<T>();
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
Referentiable* ReferentiableManager<T>::newReferentiableInternal()
{
    return newReferentiableInternal(defaultNameHint(), std::vector<std::string>());
}

template <class T>
Referentiable* ReferentiableManager<T>::newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids)
{
    LG(INFO, "ReferentiableManager<T>::newReferentiable(%s, %d guids) begin",
        (nameHint.c_str() ? nameHint.c_str() : "NULL"),
        guids.size());

    T * curAnim = NULL;

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

    curAnim = new T(this, guid, nameHint);
    if (!ComputeSessionName(curAnim))
    {
        LG(ERR, "ReferentiableManager<T>::newReferentiable : ComputeSessionName failed (uuid: %s)", guid.c_str());
        delete curAnim;
        curAnim = NULL;
        goto end;
    }

end:

    LG((curAnim ? INFO : ERR), "ReferentiableManager<T>::newReferentiable(...) returns 0x%x", curAnim);
    return curAnim;
}

template <class T>
ReferentiableNewCmd<T>::ReferentiableNewCmd(const std::string & nameHint, const std::vector<std::string> guids) :
ReferentiableNewCmdBase(nameHint, guids)
, m_manager(ReferentiableManager<T>::getInstance())
{
}
template <class T>
ReferentiableNewCmd<T>::ReferentiableNewCmd() :
ReferentiableNewCmdBase()
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
{}
ReferentiableCmdBase::~ReferentiableCmdBase()
{}

ReferentiableNewCmdBase::ReferentiableNewCmdBase(const std::string & nameHint, const std::vector<std::string> guids) :
ReferentiableCmdBase()
, m_bHasParameters(true)
, m_params({ nameHint, guids })
, m_after({ std::string(), std::string(), NULL })
{
}

ReferentiableNewCmdBase::ReferentiableNewCmdBase() :
ReferentiableCmdBase()
, m_bHasParameters(false)
, m_after({ std::string(), std::string(), NULL })
{
}

ReferentiableNewCmdBase::~ReferentiableNewCmdBase()
{}

void ReferentiableNewCmdBase::getDescription(std::string & desc)
{
    switch (getState())
    {
    case NOT_EXECUTED:
        desc.append("Waiting for command execution");
        break;
    default:
        desc.append("new Ref. \"");
        desc.append(m_after.m_sessionName);
        desc.append("\"");
        break;
    }
}

Referentiable * ReferentiableNewCmdBase::refAddr() const
{
    return m_after.m_addr;
}
bool ReferentiableNewCmdBase::doExecute()
{
    Referentiable * r = NULL;

    if (m_bHasParameters)
        r = manager()->newReferentiableInternal(m_params.m_nameHint, m_params.m_guids);
    else
        r = manager()->newReferentiableInternal();

    m_after.m_GUID = r->guid();
    m_after.m_sessionName = r->sessionName();
    m_after.m_addr = r;

    return true;
}

void ReferentiableNewCmdBase::doUndo()
{
    Referentiable * r = manager()->findByGuid(m_after.m_GUID);

    if (r)
    {
        manager()->RemoveRefInternal(r);
    }
    else
    {
        LG(ERR, "ReferentiableNewCmd<T>::doUndo : ref not found");
        assert(0);
    }
}

void ReferentiableNewCmdBase::doRedo()
{
    std::vector<std::string> guids{ m_after.m_GUID };
    Referentiable * r = manager()->newReferentiableInternal(m_after.m_sessionName, guids);

    assert(m_after.m_GUID == r->guid());
    assert(m_after.m_sessionName == r->sessionName());
}

ReferentiableDeleteCmdBase::ReferentiableDeleteCmdBase(const std::string & guid) :
ReferentiableCmdBase()
, m_bHasParameters(true)
, m_params({ guid })
, m_after({ std::string(), std::string()})
, m_addr(NULL)
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
        desc.append(m_after.m_sessionName);
        desc.append("\"");
        break;
    }
}

Referentiable * ReferentiableDeleteCmdBase::refAddr() const
{
    return m_addr;
}
bool ReferentiableDeleteCmdBase::doExecute()
{
    Referentiable * r = NULL;

    assert(m_bHasParameters);

    r = manager()->findByGuid(m_params.m_guid);

    m_after.m_GUID = r->guid();
    m_after.m_sessionName = r->sessionName();

    manager()->RemoveRefInternal(r);
    
    return true;
}

void ReferentiableDeleteCmdBase::doUndo()
{
    std::vector<std::string> guids{ m_after.m_GUID };
    Referentiable * r = manager()->newReferentiableInternal(m_after.m_sessionName, guids);

    assert(m_after.m_GUID == r->guid());
    assert(m_after.m_sessionName == r->sessionName());
}

void ReferentiableDeleteCmdBase::doRedo()
{
    Referentiable * r = manager()->findByGuid(m_after.m_GUID);

    if (r)
    {
        manager()->RemoveRefInternal(r);
    }
    else
    {
        LG(ERR, "ReferentiableNewCmd<T>::doUndo : ref not found");
        assert(0);
    }
}
