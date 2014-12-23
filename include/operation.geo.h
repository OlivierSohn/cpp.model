#pragma once

#include "updatable.h"
#include "Wire.h"
#include "wmoperation.rounded.h"
#include "wmoperation.spec.h"

namespace imajuscule
{
    class GeoOp : public Updatable
    {
    public:
        GeoOp();
        virtual ~GeoOp();

        void addRoundedWMOperation();

        void getWireSetsBounds(WireSetsBounds & wsb);

        void doUpdate() override;

        PERSISTABLE_VISITOR_HEADER_IMPL
    private:

        WireSetsBounds m_wssb;
        typedef std::pair<SpecWM*, RoundedWM*> cplxOperation;
        typedef std::vector< cplxOperation > cplxOperations;
        cplxOperations m_operations;
    };
}