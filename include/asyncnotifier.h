#pragma once

#include <vector>
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
        typedef std::vector<AsyncNotifier*> notifiers;
        static notifiers g_scheduled, g_scheduledMore;
        static bool isRunning;
        void doNotify();
    };
}
