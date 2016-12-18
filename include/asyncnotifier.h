
namespace imajuscule
{
    class AsyncNotifier final
    {
    public:
        AsyncNotifier(std::function<void(void)> & f) :
            m_f(f)
        {}
        AsyncNotifier(std::function<void(void)> && f) :
        m_f(std::move(f))
        {}

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
