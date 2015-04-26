#include "history.manager.h"
#include "referentiable.h"
#include "referentiable.manager.h"
#include <time.h>
#include "os.log.h"
#include "os.log.format.h"
#include "session.h"

#define KEY_DATE_CREA           'd' // string
#define KEY_GUID                'e' // string
#define KEY_NAME                'i' // string

using namespace imajuscule;

Referentiable::Referentiable() :
Persistable()
, m_manager(NULL)
, m_guid(std::string(""))
, m_bHasSessionName(false)
, m_bHidden(false)
, m_observableReferentiable(Observable<Event, Referentiable*>::instantiate())
{
}

Referentiable::Referentiable(ReferentiableManagerBase * manager, const std::string & guid) :
Persistable()
, m_manager(manager)
, m_guid(guid)
, m_bHasSessionName(false)
, m_bHidden(false)
, m_observableReferentiable(Observable<Event, Referentiable*>::instantiate())
{
    A(m_manager);
}

Referentiable::Referentiable(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName) :
Persistable()
, m_manager(manager)
, m_guid(guid)
, m_hintName(hintName)
, m_bHasSessionName(false)
, m_bHidden(false)
, m_observableReferentiable(Observable<Event, Referentiable*>::instantiate())
{
    A(m_manager);

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
{
    m_observableReferentiable->deinstantiate();
}

ReferentiableManagerBase * Referentiable::getManager() const
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

auto Referentiable::observableReferentiable() -> Observable<Event, Referentiable*> &
{
    return *m_observableReferentiable;
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
    A(m_bHasSessionName);
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

std::string Referentiable::extendedName() const
{
    std::string ret = m_sessionName;

    if (Referentiable * mra = mainRefAttr())
    {
        ret.append("(");
        ret.append(mra->sessionName());
        ret.append(")");
    }

    return ret;
}

Referentiable * Referentiable::mainRefAttr() const
{
    return NULL;
}


Referentiable::ReferentiablePersist::ReferentiablePersist(DirectoryPath d, FileName f, Referentiable & r):
Persistable::PersistablePersist(d, f, r)
, m_ref(r)
{
}

Referentiable::ReferentiablePersist::~ReferentiablePersist()
{
}

Referentiable::ReferentiableLoad::ReferentiableLoad(DirectoryPath d, FileName f, Referentiable & r) :
Persistable::PersistableLoad(d,f)
, m_ref(r)
{
}
Referentiable::ReferentiableLoad::~ReferentiableLoad()
{
}

eResult Referentiable::ReferentiablePersist::Save()
{
    WriteKeyData(KEY_NAME, m_ref.m_hintName);
    WriteKeyData(KEY_DATE_CREA, m_ref.m_dateOfCreation);
    WriteKeyData(KEY_GUID, m_ref.m_guid);
    
    return PersistablePersist::Save();
}

void Referentiable::ReferentiableLoad::LoadStringForKey(char key, std::string & sVal)
{
    LG(INFO, "ReferentiableLoad::LoadStringForKey(%d, %s) begin", key, (sVal.c_str() ? sVal.c_str() : "NULL"));

    switch (key)
    {
    case KEY_GUID:
        m_ref.m_guid = sVal;
        break;

    case KEY_NAME:
        m_ref.m_hintName = sVal;
        break;

    case KEY_DATE_CREA:
        m_ref.m_dateOfCreation = sVal;
        break;

    default:
        LG(ERR, "ReferentiableLoad::LoadStringForKey(%d) : unknown (or future?) tag for this object : %d", key, key);
        break;
    }

    LG(INFO, "ReferentiableLoad::LoadStringForKey(%d, %s) end", key, (sVal.c_str() ? sVal.c_str() : "NULL"));
}
