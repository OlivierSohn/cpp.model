#pragma once

#include "referentiable.manager.h"

#include "referentiables.h"

namespace imajuscule
{
    Referentiables * Referentiables::m_instance( NULL );
    Referentiables::Referentiables()
    {
        m_managers.reserve( 100 );
    }
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
        Referentiable * r = NULL;
        if(r = findRefFromGUIDLoaded(guid)) {
            return r;
        }
        
        unsigned int index;
        std::string nameHint;
        auto res = Referentiable::ReadIndexForDiskGUID(path, guid, index, nameHint);
        if_A(res)
        {
            if_A(index < m_managers.size())
            {
                // To record this in history we should have a command specializing newRefCmd for Load, with path as input)
                HistoryManagerPause p;
                
                std::vector<std::string> guids{guid};
                ReferentiableManagerBase * rm = m_managers[index];
                r = rm->newReferentiable(nameHint, guids, false, true);
                r->Load(path, guid);
                rm->observable().Notify(ReferentiableManagerBase::Event::RFTBL_ADD, r);
            }
        }
        
        return r;
    }
    Referentiable* Referentiables::findRefFromSessionNameLoaded(const std::string & sn)
    {
        for(auto man: m_managers)
        {
            if(Referentiable * ref = man->findBySessionName(sn))
                return ref;
        }
        return NULL;
    }
    Referentiable* Referentiables::findRefFromGUIDLoaded(const std::string & guid)
    {
        for(auto man: m_managers)
        {
            if(Referentiable * ref = man->findByGuid(guid)) {
                return ref;
            }
        }
        return NULL;
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
    
    void Referentiables::traverseManagers(managers::iterator & begin, managers::iterator & end)
    {
        Referentiables * i = Referentiables::getInstance();
        begin = i->m_managers.begin();
        end = i->m_managers.end();
    }
}
