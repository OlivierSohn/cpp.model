
namespace imajuscule
{
    Referentiables * Referentiables::m_instance( nullptr );
    Referentiables::Referentiables()
    {
        m_managers.reserve( 100 );
    }

    Referentiables * Referentiables::getInstance()
    {
        if (!m_instance)
        {
            Globals::make_ptr<Referentiables>(m_instance);
            if(m_instance) {
                InitializeRefManagers(*m_instance);
            }
        }
        
        return m_instance;
    }
    
    Referentiable* Referentiables::fromGUID(const DirectoryPath & path, const std::string & guid)
    {
        return getInstance()->findRefFromGUID(path, guid);
    }
    
    Referentiable* Referentiables::fromGUIDLoaded(const std::string & guid)
    {
        return getInstance()->findRefFromGUIDLoaded( guid);
    }
    
    Referentiable* Referentiables::fromSessionNameLoaded(const std::string & sn)
    {
        return getInstance()->findRefFromSessionNameLoaded(sn);
    }
    
    Referentiable* Referentiables::findRefFromGUID(const DirectoryPath & path, const std::string & guid)
    {
        if(auto r = findRefFromGUIDLoaded(guid)) {
            return r;
        }
        
        unsigned int index;
        std::string nameHint;
        auto res = Referentiable::ReadIndexForDiskGUID(path, guid, index, nameHint);
        A(res);
        A(index < m_managers.size());
        // To record this in history we should have a command specializing newRefCmd for Load, with path as input)
        HistoryManagerPause p;
        
        std::vector<std::string> guids{guid};
        ReferentiableManagerBase * rm = m_managers[index];
        auto r = rm->newReferentiable(nameHint, guids, false);
        r->Load(path, guid);
        rm->observable().Notify(ReferentiableManagerBase::Event::RFTBL_ADD, r.get());
        
        return r.release();
    }
    
    Referentiable* Referentiables::findRefFromSessionNameLoaded(const std::string & sn)
    {
        for(auto * man: m_managers) {
            if(auto ref = man->findBySessionName(sn)) {
                return ref;
            }
        }
        return {};
    }
    
    Referentiable* Referentiables::findRefFromGUIDLoaded(const std::string & guid)
    {
        for(auto * man: m_managers) {
            if(auto ref = man->findByGuid(guid)) {
                return ref;
            }
        }
        return {};
    }
    
    void Referentiables::regManager(ReferentiableManagerBase * m) {
        if(m) {
            A(m->index() == m_managers.size());
        }
        m_managers.emplace_back(m);
    }
    
    managers const & Referentiables::getManagers() {
        auto * i = Referentiables::getInstance();
        A(i);
        return i->m_managers;
    }
}
