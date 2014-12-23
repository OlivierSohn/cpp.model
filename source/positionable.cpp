#include "positionable.h"

using namespace imajuscule;


Positionable::Positionable() :
Updatable()
{
    m_position.addObserver(this);
    this->addSpec(&m_position);
}

Positionable::~Positionable()
{
    this->removeSpec(&m_position);
    m_position.removeObserver(this);
}

