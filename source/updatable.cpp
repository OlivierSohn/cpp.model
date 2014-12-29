#include "updatable.h"
#include <cassert>
#include <algorithm>
#include "os.log.h"

using namespace imajuscule;


Updatable::Updatable() :
Observable(),
m_bOneObservedChanged(true),
m_stamp(0)
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
        m_stamp++;
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

void Updatable::onObservedChanged()
{
    m_bOneObservedChanged = true;
}

unsigned int Updatable::stamp()
{
    return m_stamp;
}