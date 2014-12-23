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

void Positionable::getAbsolutePosition(float trans[3], float rot[3])
{
    if (m_parent)
        m_parent->getAbsolutePosition(trans, rot);
    else
    {
        trans[0] = 0.f;
        trans[1] = 0.f;
        trans[2] = 0.f;

        rot[0] = 0.f;
        rot[1] = 0.f;
        rot[2] = 0.f;
    }

    float localTrans[3];
    m_position.getTranslation(localTrans);
    float localRot[3];
    m_position.getRotation(localRot);

    trans[0] += localTrans[0];
    trans[1] += localTrans[1];
    trans[2] += localTrans[2];
    
    // TODO do rotation calculation
    LG(WARN, "rotation composition not implemented");
}

