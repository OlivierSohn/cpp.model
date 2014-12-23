
#pragma once

#include "persistable.h"
#include <list>
#include <memory>

namespace imajuscule
{

    class Updatable : public Persistable
    {
    public:
        // weak pointers to allow clients to not call remove(observer/spec) when an object is deleted
        //        typedef std::weak_ptr<Updatable> observer;
        //        typedef std::weak_ptr<Updatable> spec;
        typedef Updatable * observer;
        typedef Updatable * spec;

        virtual ~Updatable();

        void Update();

        void addObserver(observer );
        void removeObserver(observer);
        void addSpec(spec);
        void removeSpec(spec);

    protected:
        Updatable();

        virtual void doUpdate() = 0;

        typedef std::list< observer > observers;
        typedef std::list< spec > specs;
    private:
        void notifyObservers();
        void onObservedChanged();
        bool isConsistent() const;
        bool isObserver(observer item) const;
        bool isSpec(spec item) const;

        bool m_bOneObservedChanged;

        observers m_observers;
        specs m_specs;
    };
}
