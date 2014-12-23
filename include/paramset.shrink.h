#pragma once

#include "paramset.h"

namespace imajuscule
{
    class Shrink : public ParamSet
    {
    public:
        Shrink();
        virtual ~Shrink();

        void getParams(float & shrink);
    private:
        enum
        {
            INDEX_SHRINK = 0,
        };
    };
}