#pragma once
#include "positionable.h"
#include "operation.geo.h"

namespace imajuscule
{
    class Joint;
    class Body : public Positionable
    {
    public:
        Body(Joint * parent);
        virtual ~Body();

        void doUpdate() override;

        PERSISTABLE_VISITOR_HEADER_IMPL

    private:
        GeoOp m_geoOp;
    };
}