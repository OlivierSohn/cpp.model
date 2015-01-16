#pragma once

#include <list>
#include <functional>

namespace imajuscule
{
    class AsyncNotifier
    {
    public:
        AsyncNotifier(std::function<void(void)> & f) :
            m_f(f)
        {}
        AsyncNotifier(std::function<void(void)> && f) :
            m_f(f)
        {}
        virtual ~AsyncNotifier(){}

        static void runScheduledNotifications();
        void schedule();
        void unschedule();
    protected:
        std::function<void(void)> m_f;
    private:
        typedef std::list<AsyncNotifier*> notifiers; // we use a list because if we use a vector, in runScheduledNotifications if a notification changes the vector, the iterators are not valid anymore!
        static notifiers g_scheduled;
        void doNotify();
    };
}
