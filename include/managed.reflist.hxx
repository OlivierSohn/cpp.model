
namespace imajuscule
{
    MANAGED_REF_LIST
    ManagedRefList<Owner, T, Add, Remove>::~ManagedRefList()
    {
        A( m_regs.size() == list.size() );
        
        auto layer = list.begin();
        auto layer_end = list.end();
        
        auto registration = m_regs.begin();
        auto registration_end = m_regs.end();
        
        for(; layer != layer_end; ++layer, ++registration)
        {
            A( registration != registration_end );
            
            auto * l = layer->get();
            owner->removeSpec( l );
            l->observableReferentiable().Remove( *registration );
        }
        
        A( registration == registration_end );
    }
    
    MANAGED_REF_LIST
    void ManagedRefList<Owner, T, Add, Remove>::add(T*mc)
    {
        cmd::ManageAttr(*owner, mc, cmd::Type::TYPE_ADD);
    }
    MANAGED_REF_LIST
    void ManagedRefList<Owner, T, Add, Remove>::remove(T*mc)
    {
        cmd::ManageAttr(*owner, mc, cmd::Type::TYPE_REMOVE);
    }
    
    MANAGED_REF_LIST
    void ManagedRefList<Owner, T, Add, Remove>::remove( vregs::iterator & reg, listIterator & layer )
    {
        auto * pML = layer->get();
        owner->removeSpec(pML);
        pML->observableReferentiable().Remove( *reg );
        
        m_regs.erase( reg );
        list.erase( layer );
        
        owner->observable().Notify(PersistableEvent::OBJECT_DEFINITION_CHANGED, owner);
    }
    
    MANAGED_REF_LIST
    void ManagedRefList<Owner, T, Add, Remove>::clear()
    {
        if (!list.empty())
        {
            auto layer = list.begin();
            auto layer_end = list.end();
            
            auto registration = m_regs.begin();
            auto registration_end = m_regs.end();
            
            for(; layer != layer_end; ++layer, ++registration) {
                owner->removeSpec(layer->get());
                (*layer)->observableReferentiable().Remove( *registration );
            }
            
            A( registration == registration_end );
            
            list.clear();
            m_regs.clear();
            
            owner->observable().Notify(PersistableEvent::OBJECT_DEFINITION_CHANGED, owner);
            owner->hasNewContentForUpdate(true);
        }
    }

    
    MANAGED_REF_LIST
    void ManagedRefList<Owner, T, Add, Remove>::addInternal(T*mc)
    {
        if_A (mc)
        {
            if(unlikely(has(mc)))
            {
                A(!"Design Error : attempt to add a present item");
                return;
            }
            
            list.emplace_back( mc );
            
            m_regs.push_back( {
                mc->observableReferentiable().Register(Referentiable::Event::WILL_BE_DELETED, [this](Referentiable*r){
                    remove(static_cast<T*>(r));
                })
            });
            
            owner->addSpec(mc);
            owner->hasNewContentForUpdate(true);
            owner->observable().Notify(PersistableEvent::OBJECT_DEFINITION_CHANGED, owner);
        }
    }
    MANAGED_REF_LIST
    void ManagedRefList<Owner, T, Add, Remove>::removeInternal(T*mc)
    {
        if_A (mc)
        {
            auto layer = list.begin();
            auto layer_end = list.end();
            
            auto registration = m_regs.begin();
            auto registration_end = m_regs.end();
            
            for(; layer != layer_end; ++layer, ++registration) {
                
                A( registration != registration_end );
                if( layer->get() == mc)
                {
                    remove( registration, layer );
                    owner->hasNewContentForUpdate(true);
                    return;
                }
            }
            
            A(!"Design Error : attempt to remove an absent position");
        }
    }
    
    MANAGED_REF_LIST
    void ManagedRefList<Owner, T, Add, Remove>::set(std::vector<T*> && v)
    {
        bool changed = false;
        if ( !list.empty() ) {
            changed = true;
            clear();
        }
        
        if (!v.empty())
        {
            for ( auto t : v) {
                add( t );
            }
            
            changed = true;
        }
        if ( changed ) {
            owner->hasNewContentForUpdate(true);
        }
    }
    
    MANAGED_REF_LIST
    bool ManagedRefList<Owner, T, Add, Remove>::empty() const
    {
        return list.empty();
    }
    MANAGED_REF_LIST
    int ManagedRefList<Owner, T, Add, Remove>::size() const
    {
        return (int)list.size();
    }
    MANAGED_REF_LIST
    bool ManagedRefList<Owner, T, Add, Remove>::has(const T*mc) const
    {
        for (auto const & ml : list )
        {
            if(ml.get() == mc)
            {
                return true;
            }
        }
        return false;
    }
    
    MANAGED_REF_LIST
    const T * ManagedRefList<Owner, T, Add, Remove>::get(int index) const
    {
        return list[index].get();
    }
    
    MANAGED_REF_LIST
    T * ManagedRefList<Owner, T, Add, Remove>::edit(int index)
    {
        return list[index].get();
    }
}
