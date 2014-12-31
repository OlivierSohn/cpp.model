#include "Observable.h"
#include <cassert>
#include "os.log.h"

using namespace imajuscule;

Observable::observables Observable::m_all;

Observable::Observable() :
m_bHasNewContentForUpdate(true)
{
    m_all.insert(observables::value_type(this));
}

Observable::~Observable()
{
    m_all.erase(observables::value_type(this));
}

void Observable::traverseAll(observables::iterator & begin, observables::iterator & end)
{
    begin = m_all.begin();
    end = m_all.end();
}

void Observable::onUpdateEnd()
{
    observables::iterator it, end;
    traverseAll(it, end);
    for (; it != end; ++it)
    {
        (*it)->hasNewContentForUpdate(false);
    }
}

bool Observable::hasNewContentForUpdate() const
{
    return m_bHasNewContentForUpdate;
}

void Observable::hasNewContentForUpdate(bool bVal)
{
    m_bHasNewContentForUpdate = bVal;
}

bool Observable::isObserver(observer item) const
{
    observers::const_iterator it = m_observers.begin();
    observers::const_iterator end = m_observers.end();

    for (; it != end; ++it)
    {
        if ((*it) == item)
        {
            return true;
        }
    }
    return false;
}

void Observable::addObserver(observer item)
{
    assert(!isObserver(item));

    m_observers.push_back(item);
}

void Observable::removeObserver(observer item)
{
    m_observers.remove(item);
}

