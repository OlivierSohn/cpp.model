#pragma once

#include "updatable.h"
#include "paramset.position.h"

namespace imajuscule
{
    class Positionable : public Updatable
    {
    public:
        virtual ~Positionable();

    protected:
        Positionable();

        // The relative position
        Position m_position;
    };
}
