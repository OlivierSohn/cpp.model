#pragma once

#include "paramset.h"

namespace imajuscule
{
    class Extrude : public ParamSet
    {
    public:
        Extrude();
        virtual ~Extrude();

        void getParams(int & axis, int & length);
    private:
        enum
        {
            INDEX_AXISINDEX = 0,
            INDEX_LENGTH = 1
        };
    };
}