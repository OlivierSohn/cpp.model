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
    m_all.push_back(updatables::value_type(this));
}

Updatable::~Updatable()
{
    while(unlikely(!m_specs.empty()))
    {
        A(!"some specs needs to be cleaned up");
        removeSpec(m_specs.back());
    }
    m_all.erase(std::remove(m_all.begin(), m_all.end(), this), m_all.end());
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

    // "auto *" here was causing a crash (vector was empty)... i don't know why but "auto" fixed that
    for (auto spec : m_specs)
    {
        spec->Update();
    }

    bool bNewVal = doUpdate();

    hasNewContentForUpdate(bNewVal);

    hasBeenUpdated(true);
}

void Updatable::resetUpdateStatesRecurse()
{
    hasBeenUpdated(false);
    
    for (auto * spec : m_specs)
    {
        spec->resetUpdateStatesRecurse();
    }
}
void Updatable::resetObserversUpdateStatesRecurse()
{
    for (auto * observer : m_observers)
    {
        observer->hasBeenUpdated(false);
        observer->resetUpdateStatesRecurse();
    }
}
bool Updatable::isConsistent() const
{
    for (auto * spec : m_specs)
    {
        for (auto * observer : m_observers)
        {
            if (unlikely(observer == spec))
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
    for (auto * spec : m_specs)
    {
        if (spec == item)
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

        for (auto * observer : m_observers)
            observer->onAddRecursiveSpec(item);
    }
}

void Updatable::onAddRecursiveSpec(spec item)
{
    observableUpdatable().Notify(ADD_SPEC_RECURSE, *this, *item);

    for (auto * observer : m_observers)
        observer->onAddRecursiveSpec(item);
}
void Updatable::onRemoveRecursiveSpec(spec item)
{
    observableUpdatable().Notify(REMOVE_SPEC_RECURSE, *this, *item);

    for (auto * observer : m_observers)
        observer->onRemoveRecursiveSpec(item);
}

void Updatable::removeSpec(spec item)
{
    if (item)
    {
        item->removeObserver(this);
        m_specs.erase(std::remove(m_specs.begin(), m_specs.end(), item), m_specs.end());

        A(!isSpec(item));

        observableUpdatable().Notify(REMOVE_SPEC, *this, *item);

        for (auto * observer : m_observers)
            observer->onRemoveRecursiveSpec(item);
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
    for (auto * spec : m_specs)
    {
        spec->listSpecsRecurse(v);
    }
}

void Updatable::traverseAll(updatables::iterator & begin, updatables::iterator & end)
{
    begin = m_all.begin();
    end = m_all.end();
}

void Updatable::onUpdateEnd()
{
    for (auto * pIt: m_all)
    {
        auto & it = *pIt;
        it.hasNewContentForUpdate(false);
        it.hasBeenUpdated(false);
    }
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
    for (auto * observer : m_observers)
    {
        if (observer == item)
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
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), item), m_observers.end());
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
    for (auto * observer : m_observers)
    {
        observer->listObserversRecurse(v);
    }
}
