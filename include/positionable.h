#pragma once

#include "updatable.h"
#include "paramset.position.h"

namespace imajuscule
{
    class Positionable : public Updatable
    {
    public:
        virtual ~Positionable();

        // TODO implement a caching mechanism to not redo the calculation each time
        void getAbsolutePosition(float trans[3], float rot[3]);

    protected:
        Positionable(Positionable * parent);

        // The relative position
        Position m_position;

        Positionable * m_parent;
    };
}
