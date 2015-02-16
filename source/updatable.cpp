#include "updatable.h"
#include <algorithm>
#include "os.log.h"

using namespace imajuscule;

Updatable::updatables Updatable::m_all;


Updatable::Updatable() :
Visitable(),
m_bHasNewContentForUpdate(true),
m_bHasBeenUpdated(false),
m_observableUpdatable(Observable<Event, Updatable& /*observed*/, Updatable&/*spec*/>::instantiate())
{
    m_all.insert(updatables::value_type(this));
}

Updatable::~Updatable()
{
    m_all.erase(updatables::value_type(this));
    m_observableUpdatable->deinstantiate();
}

auto Updatable::observableUpdatable() -> Observable<Event, Updatable& /*observed*/, Updatable&/*spec*/> &
{
    return *m_observableUpdatable;

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
    if (item)
    {
        A(!isSpec(item));

        m_specs.push_back(item);
        item->addObserver(this);

        A(isConsistent());

        observableUpdatable().Notify(ADD_SPEC, *this, *item);

        observers::iterator it, end;
        traverseObservers(it, end);
        for (; it != end; ++it)
            (*it)->onAddRecursiveSpec(item);
    }
}

void Updatable::onAddRecursiveSpec(spec item)
{
    observableUpdatable().Notify(ADD_SPEC_RECURSE, *this, *item);

    observers::iterator it, end;
    traverseObservers(it, end);
    for (; it != end; ++it)
        (*it)->onAddRecursiveSpec(item);
}
void Updatable::onRemoveRecursiveSpec(spec item)
{
    observableUpdatable().Notify(REMOVE_SPEC_RECURSE, *this, *item);

    observers::iterator it, end;
    traverseObservers(it, end);
    for (; it != end; ++it)
        (*it)->onRemoveRecursiveSpec(item);
}

void Updatable::removeSpec(spec item)
{
    if (item)
    {
        item->removeObserver(this);
        m_specs.remove(item);

        A(!isSpec(item));

        observableUpdatable().Notify(REMOVE_SPEC, *this, *item);

        observers::iterator it, end;
        traverseObservers(it, end);
        for (; it != end; ++it)
            (*it)->onRemoveRecursiveSpec(item);
    }
}

void Updatable::traverseSpecs(specs::iterator & begin, specs::iterator & end)
{
    begin = m_specs.begin();
    end = m_specs.end();
}

void Updatable::listSpecs(specs & v)
{
    v.insert(v.end(), m_specs.begin(), m_specs.end());
}
void Updatable::listSpecsRecurse(specs & v)
{
    listSpecs(v);
    specs::iterator it, end;
    traverseSpecs(it, end);
    for (; it != end; ++it)
    {
        (*it)->listSpecsRecurse(v);
    }
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
    A(!isObserver(item));

    m_observers.push_back(item);
}

void Updatable::removeObserver(observer item)
{
    m_observers.remove(item);
}

void Updatable::traverseObservers(observers::iterator & begin, observers::iterator & end)
{
    begin = m_observers.begin();
    end = m_observers.end();
}

void Updatable::listObservers(observers & v)
{
    v.insert(v.end(), m_observers.begin(), m_observers.end());
}
void Updatable::listObserversRecurse(observers & v)
{
    listObservers(v);
    observers::iterator it, end;
    traverseObservers(it, end);
    for (; it != end; ++it)
    {
        (*it)->listObserversRecurse(v);
    }
}
