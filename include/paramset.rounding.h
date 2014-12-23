#pragma once

#include "paramset.h"

namespace imajuscule
{
    class Rounding : public ParamSet
    {
    public:
        Rounding();
        virtual ~Rounding();

        void getRoundings(float & roundingRatio, float & roundingStrength);
    private:
        enum
        {
            INDEX_RR = 0,
            INDEX_RS = 1
        };
    };
}