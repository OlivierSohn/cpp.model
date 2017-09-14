
namespace imajuscule
{
    class Updatable;
    typedef Updatable * spec;
    typedef std::vector< spec > specs;
    typedef Updatable * observer;
    typedef std::vector< observer > observers;

    struct UpdatePhase;
    class Updatable : public Visitable
    {
        friend struct UpdatePhase;
    public:
        enum class Event: unsigned char
        {
            ADD_SPEC,
            ADD_SPEC_RECURSE,
            REMOVE_SPEC,
            REMOVE_SPEC_RECURSE,
            
            SIZE_ENUM
        };
        virtual ~Updatable();

        void Update();
        static bool updating() { return updateAllowed; }

        void addSpec(spec);
        bool removeSpec(spec);

        // while you work with this vector, make sure to not remove or add any specs to this object
        // else you're in BIG trouble...
        std::vector< Updatable* > const & getSpecs() const { return m_specs; }
        std::vector< Updatable* > const & getObservers() const { return m_observers; }

        template<class func>
        bool forEachObserverRecurse(func f) {
            for ( auto * observer : m_observers )
            {
                if ( !observer ) { continue; }
                if ( !f(observer) ) { 
                    return false; 
                }
                if ( !observer->forEachObserverRecurse(f) ) {
                    return false;
                }
            }
            return true;
        }

        bool hasNewContentForUpdate() const { return m_bHasNewContentForUpdate; }

        void hasNewContentForUpdate(bool bVal) { m_bHasNewContentForUpdate = bVal; }

        void resetUpdateStatesRecurse();
        void resetObserversUpdateStatesRecurse();

        Observable<Event, Updatable& /*observed*/, Updatable&/*spec*/> & observableUpdatable();

        bool isSpecRecurse(Updatable const * item) const;
        bool isObserverRecurse(Updatable const * item) const;

        bool isSpec(Updatable const * item) const;
        bool isObserver(Updatable const * item) const;
        
    protected:
        Updatable();

        virtual bool doUpdate() { return hasNewContentForUpdate(); };

        enum UpdateState : unsigned char
        {
            UPDATED,
            NOTUPDATED,
            INUPDATE /* at the end so that we can increment it */
        };
        void setNotUpdated()
        {
            if(getUpdateState() == UPDATED) {
                setUpdateState(NOTUPDATED);
            }
        }

    private:
        typedef std::vector<Updatable*> updatables;
        static updatables m_all;
        static bool updateAllowed;

        UpdateState m_state : 7; // see comment in UpdateState
        bool m_bHasNewContentForUpdate : 1;

        std::vector< Updatable* > m_specs;
        std::vector< Updatable* > m_observers;

        Observable<Event, Updatable& /*observed*/, Updatable&/*spec*/> * m_observableUpdatable;

        static updatables const & traverse() { return m_all; }

        void incrementState() { Assert(m_state >= NOTUPDATED); m_state = (UpdateState)(((int)m_state)+1) ; }
        void decrementState() { Assert(m_state > NOTUPDATED); m_state = (UpdateState)(((int)m_state)-1) ; }
        UpdateState getUpdateState() const { return m_state; }
        void setUpdateState(UpdateState s) { m_state = s; }

        bool isConsistent() const;
        void onAddRecursiveSpec(spec item);
        void onRemoveRecursiveSpec(spec item);
        
        void addObserver(observer);
        void removeObserver(observer);
        
        static void onUpdateStart();
        static void onUpdateEnd();
    };
    
    struct UpdatePhase {
        UpdatePhase() {
            Updatable::onUpdateStart();
        }
        ~UpdatePhase() {
            Updatable::onUpdateEnd();
        }
    };
}
