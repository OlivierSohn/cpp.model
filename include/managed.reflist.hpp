
namespace imajuscule
{
    using regs = std::vector < FunctionInfo< Referentiable::Event >>;
    using vregs = std::vector < regs >;
    
    template <typename Owner, class T, void (Owner::*Add)(T*), bool (Owner::*Remove)(T*)>
    class ManagedRefList {
        friend Owner;
        
        Owner * owner;
        vregs m_regs;
        using listT = std::vector<intrusive_ptr<T>>;
        using listIterator = typename listT::iterator;
        listT list;

        void addInternal( T* mc){
            if(!mc) {
                return;
            }
            if(unlikely(has(mc))) {
                Assert(!"Design Error : attempt to add a present item");
                return;
            }
            list.emplace_back( mc );
            
            if(auto mcobsref = mc->observableReferentiable()) {
                m_regs.push_back( {
                    mcobsref->Register(Referentiable::Event::WILL_BE_DELETED, [this](Referentiable*r){
                        remove(safe_cast<T*>(r));
                    })
                });
            }
            owner->addSpec(mc);
            owner->hasNewContentForUpdate(true);
            owner->observable().Notify(PersistableEvent::OBJECT_DEFINITION_CHANGED, owner);
        }
        
        bool removeInternal( T* mc) {
            if(!mc) {
                return false;
            }
            auto layer = list.begin();
            auto layer_end = list.end();
            
            auto registration = m_regs.begin();
            auto registration_end = m_regs.end();
            
            for(; layer != layer_end; ++layer, ++registration) {
                
                Assert( registration != registration_end );
                if( *layer == mc)
                {
                    remove( registration, layer );
                    owner->hasNewContentForUpdate(true);
                    return true;
                }
            }
            return false;
        }
        
        using cmd = RefAttrListCmd < Owner, T, Add, Remove >;
        
        void remove(vregs::iterator & reg, listIterator & layer) {
            T & l = **layer;
            owner->removeSpec(&l);
            if(auto lobsref = l.observableReferentiable()) {
                lobsref->Remove( *reg );
            }
            m_regs.erase( reg );
            list.erase(layer);
            owner->observable().Notify(PersistableEvent::OBJECT_DEFINITION_CHANGED, owner);
        }

    public:
        using Owner_T = Owner;
        using Element_T = T;
        
        ManagedRefList(Owner * owner) : owner(owner) {}

        ~ManagedRefList() {
            Assert( m_regs.size() == list.size() );
            
            auto layer = list.begin();
            auto layer_end = list.end();
            
            auto registration = m_regs.begin();
            auto registration_end = m_regs.end();
            
            for(; layer != layer_end; ++layer, ++registration)
            {
                Assert( registration != registration_end );
                Assert(*layer);
                T & l = **layer;
                if(auto lobsref = l.observableReferentiable()) {
                    lobsref->Remove( *registration );
                }
            }
            
            Assert( registration == registration_end );
        }

        Owner_T & editOwner() { return *owner; }

        void add(T * ptr) {
            bool found;
            cmd::ManageAttr(*owner, ptr, cmd::Type::TYPE_ADD, found);
        }
        bool remove(T*mc)
        {
            bool found;
            cmd::ManageAttr(*owner, mc, cmd::Type::TYPE_REMOVE, found);
            return found;
        }
        void set(listT &&v ){
            bool changed = false;
            if ( !list.empty() ) {
                changed = true;
                clear();
            }
            
            if (!v.empty()) {
                for ( auto & t : v) {
                    add( std::move(t) );
                }
                changed = true;
            }
            if ( changed ) {
                owner->hasNewContentForUpdate(true);
            }
        }
        
        bool has(T const * mc) const
        {
            for (auto const & ml : list ) {
                if(ml == mc) {
                    return true;
                }
            }
            return false;
        }
        int size() const { return (int)list.size(); }
        bool empty() const    {
            return list.empty();
        }

        void clear()    {
            if (list.empty()) {
                return;
            }
            auto layer = list.begin();
            auto layer_end = list.end();
            
            auto registration = m_regs.begin();
            auto registration_end = m_regs.end();
            
            for(; layer != layer_end; ++layer, ++registration) {
                owner->removeSpec(&(**layer));
                if(auto lobsref = (*layer)->observableReferentiable()) {
                    lobsref->Remove( *registration );
                }
            }
            Assert( registration == registration_end );
            
            list.clear();
            m_regs.clear();
            
            owner->observable().Notify(PersistableEvent::OBJECT_DEFINITION_CHANGED, owner);
            owner->hasNewContentForUpdate(true);
        }

        T const * get(int index) const { return list[index]; }
        T * edit(int index) { return list[index]; }
        T& operator[] (const int index) { return *list[index]; }
        const T& operator[] (const int index) const { return *list[index]; }
        
        listT const & get() const { return list; }
        listT & edit() { return list; }
    };
}
