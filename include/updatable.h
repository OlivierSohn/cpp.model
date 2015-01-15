
#pragma once

#include "visitable.h"
#include <list>
#include <set>

#include "visitor.h"

namespace imajuscule
{
    class Updatable;
    typedef Updatable * spec;
    typedef std::list< spec > specs;
    typedef Updatable * observer;
    typedef std::list< observer > observers;

    class Updatable : public Visitable
    {
    public:
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

    protected:
        Updatable();

        virtual bool doUpdate() { return hasNewContentForUpdate(); };

        void hasBeenUpdated(bool);
        bool hasBeenUpdated() const;

    private:
        typedef std::set<Updatable*> updatables;
        static updatables m_all;

        bool m_bHasBeenUpdated;
        bool m_bHasNewContentForUpdate;

        specs m_specs;
        observers m_observers;

        static void traverseAll(updatables::iterator & begin, updatables::iterator & end);
        bool isObserver(observer item) const;
        void resetUpdateStates();

        bool isConsistent() const;
        bool isSpec(spec item) const;        
    };
}
