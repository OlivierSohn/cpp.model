#include "positionable.h"
#include "os.log.h"

using namespace imajuscule;


Positionable::Positionable(Positionable * parent) :
Updatable(),
m_parent(parent)
{
    this->addSpec(&m_position);
    
    if ( parent )
        this->addSpec(parent);
}

Positionable::~Positionable()
{
    if (m_parent)
        this->removeSpec(m_parent);

    this->removeSpec(&m_position);
}

void Positionable::setParent(Positionable * parent)
{
    if ( m_parent )
        this->removeSpec(m_parent);

    m_parent = parent;
}

Positionable * Positionable::parent()
{
    return m_parent;
}

void Positionable::getAbsolutePosition(float trans[3], float rot[3])
{
    float localTrans[3];
    m_position.getTranslation(localTrans);
    float localRot[3];
    m_position.getRotation(localRot);

    if (m_parent)
    {
        m_parent->getAbsolutePosition(trans, rot);

        // TODO do rotation calculation
        LG(WARN, "composition not implemented");
    }

    memcpy(trans, localTrans, sizeof(localTrans));
    memcpy(rot, localRot, sizeof(localRot));
}

