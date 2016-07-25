
#pragma once

#include "visitable.h"
#include <list>
#include <set>

#include "visitor.h"
#include "observable.h"

namespace imajuscule
{
    class Updatable;
    typedef Updatable * spec;
    typedef std::vector< spec > specs;
    typedef Updatable * observer;
    typedef std::vector< observer > observers;

    class Updatable;
    class Updatable : public Visitable
    {
    public:
        enum Event
        {
            ADD_SPEC,
            ADD_SPEC_RECURSE,
            REMOVE_SPEC,
            REMOVE_SPEC_RECURSE
        };
        virtual ~Updatable();

        void Update();

        void addSpec(spec);
        void removeSpec(spec);

        // while you work with this vector, make sure to not remove or add any specs to this object
        // else you're in BIG trouble...
        specs const & getSpecs() const { return m_specs; }

        void addObserver(observer);
        void removeObserver(observer);

        template<class func>
        bool forEachObserverRecurse(func & f) {
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

        static void onUpdateEnd();
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

        enum UpdateState { UPDATED, NOTUPDATED, INUPDATE /* at the end so that we can increment it */ };
        void setNotUpdated()
        {
            if(getUpdateState() == UPDATED) {
                setUpdateState(NOTUPDATED);
            }
        }

    private:
        typedef std::vector<Updatable*> updatables;
        static updatables m_all;

        UpdateState m_state;
        bool m_bHasNewContentForUpdate;

        specs m_specs;
        observers m_observers;

        Observable<Event, Updatable& /*observed*/, Updatable&/*spec*/> * m_observableUpdatable;

        static updatables const & traverse() { return m_all; }

        void incrementState() { A(m_state >= NOTUPDATED); m_state = (UpdateState)(((int)m_state)+1) ; }
        void decrementState() { A(m_state > NOTUPDATED); m_state = (UpdateState)(((int)m_state)-1) ; }
        UpdateState getUpdateState() const { return m_state; }
        void setUpdateState(UpdateState s) { m_state = s; }

        bool isConsistent() const;
        void onAddRecursiveSpec(spec item);
        void onRemoveRecursiveSpec(spec item);
    };
}
