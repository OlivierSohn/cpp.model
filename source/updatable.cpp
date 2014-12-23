#include "updatable.h"
#include <cassert>
#include "os.log.h"

using namespace imajuscule;


Updatable::Updatable() :
Persistable(),
m_bOneObservedChanged(true)
{
}

Updatable::~Updatable()
{

}

void Updatable::Update()
{
    specs::iterator it = m_specs.begin();
    specs::iterator end = m_specs.end();

    for (; it != end; ++it)
    {
        (*it)->Update();
    }

    //The Updatable observes each of its specs
    if (m_bOneObservedChanged)
    {
        doUpdate();
        m_bOneObservedChanged = false;
    }
}

bool Updatable::isConsistent() const
{
    specs::const_iterator it = m_specs.begin();
    specs::const_iterator end = m_specs.end();

    for (; it != end; ++it)
    {
        observers::const_iterator it2 = m_observers.begin();
        observers::const_iterator end2 = m_observers.end();

        for (; it2 != end2; ++it2)
        {
            if ((*it) == (*it2))
            {
                LG(ERR, "Updatable::isConsistent : a spec is also an observer");
                return false;
            }
        }
    }

    return true;
}

bool Updatable::isObserver(observer item) const
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

bool Updatable::isSpec(spec item) const
{
    specs::const_iterator it = m_specs.begin();
    specs::const_iterator end = m_specs.end();

    for (; it != end; ++it)
    {
        if ((*it) == item)
        {
            return true;
        }
    }
    return false;
}

void Updatable::addObserver(observer item)
{
    assert(!isObserver(item));

    m_observers.push_back(item);
    
    assert(isConsistent());
}

void Updatable::removeObserver(observer item)
{
    m_observers.remove(item);
    
    assert(isConsistent());
}

void Updatable::addSpec(spec item)
{
    assert(!isSpec(item));

    m_specs.push_back(item);
    item->addObserver(this);

    assert(isConsistent());
}

void Updatable::removeSpec(spec item)
{
    item->removeObserver(this);
    m_specs.remove(item);

    assert(!isSpec(item));
}

void Updatable::notifyObservers()
{
    observers::const_iterator it = m_observers.begin();
    observers::const_iterator end = m_observers.end();

    for (; it != end; ++it)
    {
        (*it)->onObservedChanged();
    }
}

void Updatable::onObservedChanged()
{
    m_bOneObservedChanged = true;
}

