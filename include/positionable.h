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

        PERSISTABLE_VISITOR_HEADER_IMPL

    protected:
        // The relative position
        Position m_position;

        void setParent(Positionable * parent); // protected because we don't want a body to be a parent of a joint
        Positionable * parent();

        Positionable(Positionable * parent);
    private:
        Positionable * m_parent;
    };
}
