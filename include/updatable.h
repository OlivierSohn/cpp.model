
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
        void traverseSpecs(specs::iterator & begin, specs::iterator & end);
        void listSpecs(specs &);
        void listSpecsRecurse(specs &);

        void addObserver(observer);
        void removeObserver(observer);
        void traverseObservers(observers::iterator & begin, observers::iterator & end);
        void listObservers(observers &);
        void listObserversRecurse(observers &);

        bool hasNewContentForUpdate() const;
        void hasNewContentForUpdate(bool);

        static void onUpdateEnd();
        void resetUpdateStatesRecurse();
        void resetObserversUpdateStatesRecurse();

        Observable<Event, Updatable& /*observed*/, Updatable&/*spec*/> & observableUpdatable();

        bool isSpecRecurse(spec item) const;
        bool isObserverRecurse(spec item) const;

    protected:
        Updatable();

        virtual bool doUpdate() { return hasNewContentForUpdate(); };

        void hasBeenUpdated(bool);
        bool hasBeenUpdated() const;

    private:
        typedef std::vector<Updatable*> updatables;
        static updatables m_all;

        bool m_bHasBeenUpdated;
        bool m_bHasNewContentForUpdate;

        specs m_specs;
        observers m_observers;

        Observable<Event, Updatable& /*observed*/, Updatable&/*spec*/> * m_observableUpdatable;

        static void traverseAll(updatables::iterator & begin, updatables::iterator & end);
        bool isObserver(observer item) const;

        bool isConsistent() const;
        bool isSpec(spec item) const;
        void onAddRecursiveSpec(spec item);
        void onRemoveRecursiveSpec(spec item);
    };
}
