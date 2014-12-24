#include "Observable.h"
#include <cassert>
#include "os.log.h"

using namespace imajuscule;


Observable::Observable()
{
}

Observable::~Observable()
{

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

void Observable::notifyObservers()
{
    observers::const_iterator it = m_observers.begin();
    observers::const_iterator end = m_observers.end();

    for (; it != end; ++it)
    {
        (*it)->notifyObservers();

        (*it)->onObservedChanged();
    }
}
