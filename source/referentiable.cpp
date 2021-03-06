
constexpr int KEY_DATE_CREA           = 'd' ;// string
constexpr int KEY_GUID                = 'e' ;// string
constexpr int KEY_NAME                = 'i' ;// string
constexpr int KEY_MANAGER_INDEX       =  -10;

using namespace imajuscule;

Referentiable::Referentiable() :
Persistable()
, m_manager(nullptr)
, m_guid(std::string(""))
, m_observableReferentiable(Observable<Event, Referentiable*>::instantiate())
{
}

Referentiable::Referentiable(ReferentiableManagerBase * manager, std::string && guid) :
Persistable()
, m_manager(manager)
, m_guid(std::move(guid))
, m_observableReferentiable(Observable<Event, Referentiable*>::instantiate())
{
    Assert(m_manager);
}

Referentiable::Referentiable(ReferentiableManagerBase * manager, std::string && guid, const std::string & hintName) :
Persistable()
, m_manager(manager)
, m_guid(std::move(guid))
, m_hintName(hintName)
, m_observableReferentiable(Observable<Event, Referentiable*>::instantiate())
{
    Assert(m_manager);

    WriteCurrentDate(m_dateOfCreation);
}

void Referentiable::deleteObservableReferentiable()
{
    m_observableReferentiable->deinstantiate();
    m_observableReferentiable = 0;
}

namespace imajuscule {
    intrusive_ptr<Referentiable> instantiate(ReferentiableManagerBase * rm, const std::string & hintName) {
        return intrusive_ptr<Referentiable>{rm->newReferentiable(hintName, true).release()};
    }

    intrusive_ptr<Referentiable> instantiate(ReferentiableManagerBase * rm) {
        return instantiate(rm, std::string(rm->defaultNameHint()));
    }
}

void Referentiable::deinstantiate() {
    getManager()->RemoveRef(this);
}

auto Referentiable::observableReferentiable() -> Observable<Event, Referentiable*> * {
    return m_observableReferentiable; // might be zero if it is being destroyed
}

const std::string & Referentiable::guid() const {
    return m_guid;
}

const std::string & Referentiable::hintName() const {
    return m_hintName;
}

const std::string & Referentiable::sessionName() const {
    return m_sessionName;
}

void Referentiable::setSessionName(const std::string & sn) {
    //LG(INFO, "Referentiable::setSessionName(%s)", sn.empty()?"nullptr" : sn.c_str());
    m_sessionName = sn;
}

const std::string & Referentiable::creationDate() const {
    return m_dateOfCreation;
}

std::string Referentiable::extendedName() const {
    auto ret = m_sessionName;
    if (auto mra = mainRefAttr()) {
        ret += "(" + mra->sessionName() + ")";
    }
    return ret;
}

Referentiable* Referentiable::mainRefAttr() const {
    return {};
}

IMPL_PERSIST3(Referentiable, Persistable,
             
             ReferentiableManagerBase * rm = m_Referentiable.getManager();
             Assert(rm);
             WriteKeyData(KEY_MANAGER_INDEX, (int32_t)rm->index());
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
    Assert(bFound);
    return bFound;
}


