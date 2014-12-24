
#pragma once

#include <list>
#include <memory>

namespace imajuscule
{
    class Observable
    {
    public:
        typedef Observable * observer;

        virtual ~Observable();

        void addObserver(observer );
        void removeObserver(observer);
    protected:
        Observable();

        void notifyObservers();

        typedef std::list< observer > observers;
        observers m_observers;
    private:
        bool isObserver(observer item) const;

        virtual void onObservedChanged() {}
    };
}
