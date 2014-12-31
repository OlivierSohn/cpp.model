
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

        bool hasNewContentForUpdate() const;
        void hasNewContentForUpdate(bool);
    protected:
        Observable();

        typedef std::list< observer > observers;
        observers m_observers;

    private:
        bool isObserver(observer item) const;

        bool m_bHasNewContentForUpdate;
    };
}
