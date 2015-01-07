
#pragma once

#include <list>
#include <set>

#include "visitable.h"

namespace imajuscule
{
    // TODO Observable : notion has changed, m_observers is the backlink of Updatable::spec but has no purpose...
    class Observable : public Visitable
    {
    public:
        typedef Observable * observer;

        virtual ~Observable();

        void addObserver(observer );
        void removeObserver(observer);

        bool hasNewContentForUpdate() const;
        void hasNewContentForUpdate(bool);

        static void onUpdateEnd();
    protected:
        Observable();

        typedef std::list< observer > observers;
        observers m_observers;

        void hasBeenUpdated(bool);
        bool hasBeenUpdated() const;

    private:
        typedef std::set<Observable*> observables;
        static observables m_all;

        static void traverseAll(observables::iterator & begin, observables::iterator & end);
        bool isObserver(observer item) const;
        void resetUpdateStates();

        bool m_bHasBeenUpdated;
        bool m_bHasNewContentForUpdate;
    };
}
