#include "asyncnotifier.h"
#include "os.log.h"

using namespace imajuscule;

AsyncNotifier::notifiers AsyncNotifier::g_scheduled;
AsyncNotifier::notifiers AsyncNotifier::g_scheduledMore;
bool AsyncNotifier::isRunning = false;

void AsyncNotifier::schedule()
{
    if(isRunning)
        g_scheduledMore.push_back(this);
    else
        g_scheduled.push_back(this);
}

void AsyncNotifier::unschedule()
{
    for(auto & p : g_scheduled)
    {
        if(this == p)
        {
            p = NULL;
            return;
        }
    }
    for(auto & p : g_scheduledMore)
    {
        if(this == p)
        {
            p = NULL;
            return;
        }
    }
}

void AsyncNotifier::runScheduledNotifications()
{
    isRunning = true;
    
    do
    {
        for (auto * p : g_scheduled)
        {
            if( p )
                p->doNotify();
        }
        
        g_scheduled.clear();
        g_scheduled.swap(g_scheduledMore);
    }
    while(!g_scheduled.empty());
    
    isRunning = false;
}


void AsyncNotifier::doNotify()
{
    m_f();
}