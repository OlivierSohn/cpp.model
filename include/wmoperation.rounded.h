#pragma once

#include "updatable.h"
#include "wmoperation.h"
#include "paramset.rounding.h"
#include "Wire.h"

class WireModel;

namespace imajuscule
{
    class SpecWM;

    class RoundedWM : public Updatable, public WMOperation
    {
    public:

        RoundedWM(SpecWM *); // NOT owner of SpecWM
        virtual ~RoundedWM();

        void doUpdate() override;

        PERSISTABLE_VISITOR_HEADER_IMPL
    private:
        Rounding m_rounding;
        SpecWM * m_spec;
    };
}