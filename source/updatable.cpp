// stl includes
#include <algorithm>

// model includes
#include "updatable.h"
#ifndef NDEBUG
#include "raii.hpp"
#endif

// os.log includes
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
        if(auto s = m_specs.back())
        {
            A(!"some specs needs to be cleaned up");
            removeSpec(s);
        }
        else
        {
            m_specs.erase(std::remove(m_specs.begin(), m_specs.end(), (void*)0), m_specs.end());
        }
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

    {
        // vector can change size (grow but not shrink) 
        // so we access elements by [] instead of iterators

        int size = (int)m_specs.size();
        for ( int i = 0; ; i++ )
        {
            if ( i >= size )
            {
                auto s = (int) m_specs.size();
                if ( s == size )
                {
                    break;
                }

                size = s;
                if ( i >= size )
                {
                    break;
                }
            }

            if( auto s = m_specs[i] )
            {
                s->Update();
            }
        }
        
        m_specs.erase(std::remove(m_specs.begin(), m_specs.end(), (void*)0), m_specs.end());
    }

    bool bNewVal = doUpdate();

    hasNewContentForUpdate(bNewVal);

    hasBeenUpdated(true);
}

void Updatable::resetUpdateStatesRecurse()
{
    hasBeenUpdated(false);
    for ( auto * u : m_specs ) {
        u->resetUpdateStatesRecurse();
    }
}
void Updatable::resetObserversUpdateStatesRecurse()
{
    for (auto * observer : m_observers)
    {
        if(observer)
        {
            observer->hasBeenUpdated(false);
            observer->resetObserversUpdateStatesRecurse();
        }
    }
}
bool Updatable::isConsistent() const
{
    for (auto * spec : m_specs)
    {
        for (auto * observer : m_observers)
        {
            if (unlikely(observer && spec && (observer == spec)))
            {
                LG(ERR, "Updatable::isConsistent : a spec is also an observer");
                return false;
            }
        }
    }

    return true;
}

bool Updatable::isSpecRecurse(spec item) const
{
    for (auto * spec : m_specs)
    {
        if(!spec)
        {
            continue;
        }
        
        if (spec == item)
        {
            return true;
        }
        if( spec->isSpecRecurse(item))
        {
            return true;
        }
    }
    return false;
}

bool Updatable::isSpec(spec item) const
{
    for (auto * spec : m_specs)
    {
        if(!spec)
        {
            continue;
        }

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
        {
            if(observer)
            {
                observer->onAddRecursiveSpec(item);
            }
        }
    }
}

void Updatable::onAddRecursiveSpec(spec item)
{
    observableUpdatable().Notify(ADD_SPEC_RECURSE, *this, *item);

    for (auto * observer : m_observers)
    {
        if(observer)
        {
            observer->onAddRecursiveSpec(item);
        }
    }
}
void Updatable::onRemoveRecursiveSpec(spec item)
{
    observableUpdatable().Notify(REMOVE_SPEC_RECURSE, *this, *item);

    for (auto * observer : m_observers)
    {
        if(observer)
        {
            observer->onRemoveRecursiveSpec(item);
        }
    }
}

void Updatable::removeSpec(spec item)
{
    if (item)
    {
        item->removeObserver(this);
        
        auto f = std::find(m_specs.begin(), m_specs.end(), item);
        if(f != m_specs.end())
        {
            *f = NULL;
        }
        else
        {
            A(0);
        }
        
        A(!isSpec(item));

        observableUpdatable().Notify(REMOVE_SPEC, *this, *item);

        for (auto * observer : m_observers)
        {
            if(observer)
            {
                observer->onRemoveRecursiveSpec(item);
            }
        }
    }
}

auto Updatable::getSpecs() const -> specs const &
{
    return m_specs;
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

bool Updatable::isObserverRecurse(observer item) const
{
    for (auto * observer : m_observers)
    {
        if(!observer)
        {
            continue;
        }
        
        if (observer == item)
        {
            return true;
        }
        if(observer->isObserverRecurse(item))
        {
            return true;
        }
    }
    return false;
}

bool Updatable::isObserver(observer item) const
{
    for (auto * observer : m_observers)
    {
        if(!observer)
        {
            continue;
        }
     
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
        if(!observer)
        {
            continue;
        }
        
        observer->listObserversRecurse(v);
    }
}
