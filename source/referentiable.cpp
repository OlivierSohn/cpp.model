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
#define KEY_MANAGER_INDEX        -10

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
    //LG(INFO,"delete %s %s destr", m_guid.c_str(), m_sessionName.c_str());

    for(auto r:m_others)
        r->deinstantiate();

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
    return rm->newReferentiable(hintName, true);
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

IMPL_PERSIST3(Referentiable, Persistable,
             
             ReferentiableManagerBase * rm = m_Referentiable.getManager();
              if_A(rm) {
                WriteKeyData(KEY_MANAGER_INDEX, (int32_t)rm->index());
              }
             WriteKeyData(KEY_NAME, m_Referentiable.m_hintName);
             WriteKeyData(KEY_DATE_CREA, m_Referentiable.m_dateOfCreation);
             WriteKeyData(KEY_GUID, m_Referentiable.m_guid);
    
             ,
             
             case KEY_GUID:
             m_Referentiable.m_guid = str;
             break;
             case KEY_NAME:
             m_Referentiable.m_hintName = str;
             break;
             case KEY_DATE_CREA:
             m_Referentiable.m_dateOfCreation = str;
             break;

              ,
              ,
              
              case KEY_MANAGER_INDEX:
              break;
             );

Referentiable::ReferentiableIndexLoad::ReferentiableIndexLoad( DirectoryPath d, FileName f) :
KeysLoad(d,f,false)
, m_bFound(false)
, m_uiIndex(0)
{
    ReadAllKeys();
}
Referentiable::ReferentiableIndexLoad::~ReferentiableIndexLoad()
{}
void Referentiable::ReferentiableIndexLoad::LoadInt32ForKey(char key, int32_t i)
{
    switch(key)
    {
        case KEY_MANAGER_INDEX:
            m_bFound = true;
            m_uiIndex = i;
            break;
            
        default:
            break;
    }
}
void Referentiable::ReferentiableIndexLoad::LoadStringForKey(char key, std::string & str)
{
    switch (key)
    {
        case KEY_NAME:
            m_hintName = str;
            break;
        case KEY_DATE_CREA:
            m_dateOfCreation = str;
            break;
        default:
            break;
    }
}

bool Referentiable::ReferentiableIndexLoad::found(unsigned int &index, std::string & sHintName)
{
    index = m_uiIndex;
    sHintName = m_hintName;
    return m_bFound;
}

const std::string & Referentiable::ReferentiableIndexLoad::dateCrea()
{
    return m_dateOfCreation;
}

bool Referentiable::ReadIndexForDiskGUID(const DirectoryPath & path, const std::string & guid, unsigned int &index, std::string & sHintName)
{
    bool bFound = false;
    Referentiable::ReferentiableIndexLoad l(path, guid);
    bFound = l.found(index, sHintName);
    A(bFound);
    return bFound;
}

void Referentiable::registerSource( Referentiable& source )
{
    //LG(INFO,"reg s s:%x t:%x", &source, this);
    m_sources.push_back(&source);
    observableReferentiable().Notify(SOURCES_CHANGED, this);
}
void Referentiable::registerTarget( Referentiable& target )
{
    //LG(INFO,"reg t s:%x t:%x", this, &target);
    m_targets.push_back(&target);
    observableReferentiable().Notify(TARGETS_CHANGED, this);
}

void Referentiable::unRegisterTarget( Referentiable& target )
{
    //LG(INFO,"unr t s:%x t:%x", this, &target);
    m_targets.erase(std::remove(m_targets.begin(), m_targets.end(), &target), m_targets.end());
    observableReferentiable().Notify(TARGETS_CHANGED, this);
}
void Referentiable::unRegisterSource( Referentiable& source )
{
    //LG(INFO,"unr s s:%x t:%x", &source, this);
    m_sources.erase(std::remove(m_sources.begin(), m_sources.end(), &source), m_sources.end());
    observableReferentiable().Notify(SOURCES_CHANGED, this);
}

void Referentiable::traverseTargets(refs::iterator & begin, refs::iterator & end)
{
    begin = m_targets.begin();
    end = m_targets.end();
}
void Referentiable::traverseSources(refs::iterator & begin, refs::iterator & end)
{
    begin = m_sources.begin();
    end = m_sources.end();
}

size_t Referentiable::countTargets()
{
    return m_targets.size();
}
size_t Referentiable::countSources()
{
    return m_sources.size();
}



