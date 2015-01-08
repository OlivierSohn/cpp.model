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

ReferentiableManager::ReferentiableManager():
Visitable()
{}

ReferentiableManager::~ReferentiableManager()
{
    m_observable.Notify(Event::MANAGER_DELETE, NULL);

    {
        guidsToRftbls::iterator it = m_guidsToRftbls.begin();
        for (; it != m_guidsToRftbls.end(); ++it)
        {
            delete (it->second);
        }
    }
}

Observable<ReferentiableManager::Event, Referentiable*/*, bool*/> & ReferentiableManager::observable()
{
    return m_observable;
}

bool ReferentiableManager::Register(Referentiable * r, const std::string & sessionName)
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
                LG(ERR, "ReferentiableManager::Register : an element was not found in guid map but found in session names map!");
                assert(0);
            }

            m_observable.Notify(Event::RFTBL_CREATE, r/*, false*/);
        }
    }
    else
    {
        LG(ERR, "ReferentiableManager::Register : NULL param");
        assert(0);
    }

    return bRet;
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

void ReferentiableManager::ListReferentiablesByCreationDate(referentiables& vItems)
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

Referentiable * ReferentiableManager::findByGuid(const std::string & guid)
{
    Referentiable * pRet = NULL;
    guidsToRftbls::iterator it = m_guidsToRftbls.find(guidsToRftbls::key_type(guid));
    if (it != m_guidsToRftbls.end())
        pRet = it->second;
    return pRet;
}
// session name is unique per-session
Referentiable * ReferentiableManager::findBySessionName(const std::string & sessionName)
{
    Referentiable * pRet = NULL;
    snsToRftbls::iterator it = m_snsToRftbls.find(snsToRftbls::key_type(sessionName));
    if (it != m_snsToRftbls.end())
        pRet = it->second;
    return pRet;
}

bool ReferentiableManager::ComputeSessionName(Referentiable * r)
{
    bool bRet = false;

    if (r)
    {
        std::string hint = r->hintName();
        std::string sessionName = hint;
        Referentiable * r2 = findBySessionName(sessionName);
        while (r2)
        {
            sessionName.append("1");
            r2 = findBySessionName(sessionName);
        }
        bRet = Register(r, sessionName);
    }
    else
    {
        LG(ERR, "ReferentiableManager::ComputeSessionName: r is NULL");
        assert(0);
    }

    return bRet;
}

void ReferentiableManager::generateGuid(std::string & sGuid)
{
    LG(INFO, "ReferentiableManager::generateGuid : begin");

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
    LG(ERR, "ReferentiableManager::generateGuid : on android, the guid should be generated in java");
#else
    uuid_t uu;
    uuid_generate(uu);
    char uuid[37];
    uuid_unparse(uu, uuid);
    sGuid.assign(uuid);
#endif

    LG(INFO, "ReferentiableManager::generateGuid returns %s", sGuid.c_str());
}


