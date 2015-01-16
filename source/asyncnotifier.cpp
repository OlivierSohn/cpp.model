#include "asyncnotifier.h"
#include <cassert>

using namespace imajuscule;

AsyncNotifier::notifiers AsyncNotifier::g_scheduled;

void AsyncNotifier::schedule()
{
    g_scheduled.push_back(this);
}

void AsyncNotifier::unschedule()
{
    g_scheduled.remove(this);
}

void AsyncNotifier::runScheduledNotifications()
{
    notifiers::iterator it, end;

    it = g_scheduled.begin();
    end = g_scheduled.end();

    for (; it != end; ++it)
    {
        // this call can add elemnts to g_scheduled, it is ok because for a std::list, 
        // the iterators are not invalidated by insertion (not it nor end)
        (*it)->doNotify();
    }

    // assert that end iterator didn't change (even if size of list has changed)
    assert(end == g_scheduled.end());

    g_scheduled.clear();
}


void AsyncNotifier::doNotify()
{
    m_f();
}