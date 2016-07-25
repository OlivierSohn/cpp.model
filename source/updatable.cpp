//#define LOG_UPDATES

// stl includes
#include <algorithm>

#if defined (LOG_UPDATES)
#include <string>
#endif

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
#if defined (LOG_UPDATES)
    static int level = 0;
    struct Level {
        Level(int&l) : l(l) { l++; };
        ~ Level() {l--;}
    private:
        int&l;
    };
    Level l(level);
    std::string white;
    for(int i=0; i<level; i++) {
        white += "  ";
    }
    LG(INFO, "%s updating %p", white.c_str(), this);
#endif
    if (hasBeenUpdated())
        return;

    {
        // vector can change size
        // so we access elements by [] instead of iterators

        bool oneNull = false;
        for ( int i = 0; i < (int)m_specs.size(); i++ )
        {
            if( auto s = m_specs[i] )
            {
                s->Update();
            } else {
                oneNull = true;
            }
        }
        
        if ( oneNull ) {
            m_specs.erase(std::remove(m_specs.begin(), m_specs.end(), (void*)0), m_specs.end());
        }
    }

    bool bNewVal = doUpdate();

    hasNewContentForUpdate(bNewVal);

    hasBeenUpdated(true);
}

void Updatable::resetUpdateStatesRecurse()
{
    hasBeenUpdated(false);
    for ( auto * u : m_specs ) {
        if ( u ) {
            u->resetUpdateStatesRecurse();
        }
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

bool Updatable::isSpecRecurse(Updatable const * item) const
{
    for (auto const * spec : m_specs)
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

bool Updatable::isSpec(Updatable const * item) const
{
    for (auto const * spec : m_specs)
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
        A(!isObserver(item));

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

void Updatable::onUpdateEnd()
{
    for (auto * pIt: m_all)
    {
        pIt->hasNewContentForUpdate(false);
        pIt->hasBeenUpdated(false);
    }
}


bool Updatable::isObserverRecurse(Updatable const * item) const
{
    for (auto const * observer : m_observers)
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

bool Updatable::isObserver(Updatable const * item) const
{
    for (auto const * observer : m_observers)
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

