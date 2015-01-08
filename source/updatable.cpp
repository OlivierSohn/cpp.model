#include "updatable.h"
#include <cassert>
#include <algorithm>
#include "os.log.h"

using namespace imajuscule;

Updatable::updatables Updatable::m_all;


Updatable::Updatable() :
Visitable(),
m_bHasNewContentForUpdate(true),
m_bHasBeenUpdated(false)
{
    m_all.insert(updatables::value_type(this));
}

Updatable::~Updatable()
{
    m_all.erase(updatables::value_type(this));
}

void Updatable::Update()
{
    if (hasBeenUpdated())
        return;

    specs::iterator it = m_specs.begin();
    specs::iterator end = m_specs.end();

    for (; it != end; ++it)
    {
        (*it)->Update();
    }

    bool bOldVal = hasNewContentForUpdate();

    bool bNewVal = doUpdate();

    if ( bOldVal != bNewVal )
        hasNewContentForUpdate(bNewVal);

    hasBeenUpdated(true);
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

void Updatable::traverseSpecs(specs::iterator & begin, specs::iterator & end)
{
    begin = m_specs.begin();
    end = m_specs.end();
}

void Updatable::traverseAll(updatables::iterator & begin, updatables::iterator & end)
{
    begin = m_all.begin();
    end = m_all.end();
}

void Updatable::onUpdateEnd()
{
    updatables::iterator it, end;
    traverseAll(it, end);
    for (; it != end; ++it)
    {
        (*it)->resetUpdateStates();
    }
}

void Updatable::resetUpdateStates()
{
    hasNewContentForUpdate(false);
    hasBeenUpdated(false);
}

bool Updatable::hasNewContentForUpdate() const
{
    return m_bHasNewContentForUpdate;
}

void Updatable::hasNewContentForUpdate(bool bVal)
{
    m_bHasNewContentForUpdate = bVal;
}

bool Updatable::hasBeenUpdated() const
{
    return m_bHasBeenUpdated;
}

void Updatable::hasBeenUpdated(bool bVal)
{
    m_bHasBeenUpdated = bVal;
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

void Updatable::addObserver(observer item)
{
    assert(!isObserver(item));

    m_observers.push_back(item);
}

void Updatable::removeObserver(observer item)
{
    m_observers.remove(item);
}

