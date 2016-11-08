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
bool Updatable::updateAllowed = false;


Updatable::Updatable() :
Visitable(),
m_bHasNewContentForUpdate(true),
m_state(NOTUPDATED),
m_observableUpdatable(Observable<Event, Updatable& /*observed*/, Updatable&/*spec*/>::instantiate())
{
    m_all.push_back(updatables::value_type(this));
}

Updatable::~Updatable()
{
    while(!m_specs.empty())
    {
        if(auto s = m_specs.back())
        {
            removeSpec(s);
        }
        else
        {
            m_specs.erase(std::remove(m_specs.begin(), m_specs.end(), (void*)0), m_specs.end());
        }
    }
    
    while(!m_observers.empty())
    {
        if(auto s = m_observers.back())
        {
            s->removeSpec(this);
        }
        else
        {
            m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), (void*)0), m_observers.end());
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
    
    // The Update / ForceUpdate calls
    // should never happen outside the
    // update region, this leads to
    // nasty bugs because we think something
    // is up-to-date and it is not...
    //
    // ... so instead of updating something immediately
    // we  need to setup specs correctly and rely on update
    A(updateAllowed);
    
    if (UPDATED == getUpdateState())
        return;
    // we can have inner updates : for example if a param is mixed, when updating the param it updates the mixer which in turns (for the first step to ensure param has a value) updates the param

    incrementState(); // the first time, from NOTUPDATED to INUPDATE
    
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
        
        A(m_state >= INUPDATE);
        if ( oneNull && m_state == INUPDATE ) {
            m_specs.erase(std::remove(m_specs.begin(), m_specs.end(), (void*)0), m_specs.end());
        }
    }

    bool bNewVal = doUpdate();
    
    decrementState();
    
    if(m_state == NOTUPDATED) {
        m_state = UPDATED;

        hasNewContentForUpdate(bNewVal);
    } else if(bNewVal) {
        hasNewContentForUpdate(true);
    }
}

void Updatable::resetUpdateStatesRecurse()
{
    if(m_state == UPDATED) {
        m_state = NOTUPDATED;
    }
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
            if(observer->m_state == UPDATED) {
                observer->m_state = NOTUPDATED;
            }
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
    if (!item)
    {
        return;
    }
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

bool Updatable::removeSpec(spec item)
{
    if (!item) {
        return false;
    }
    
    auto f = std::find(m_specs.begin(), m_specs.end(), item);
    if(f == m_specs.end()) {
        return false;
    }

    *f = nullptr;
    item->removeObserver(this);
    observableUpdatable().Notify(REMOVE_SPEC, *this, *item);
    
    for (auto * observer : m_observers)
    {
        if(observer)
        {
            observer->onRemoveRecursiveSpec(item);
        }
    }
    
    A(!isSpec(item));
    return true;
}
void Updatable::onUpdateStart() {
    updateAllowed = true;
}

void Updatable::onUpdateEnd()
{
    updateAllowed = false;
    
    for (auto * pIt: m_all)
    {
        pIt->hasNewContentForUpdate(false);
        pIt->setNotUpdated();
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

