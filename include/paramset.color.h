#pragma once

#include "paramset.h"

namespace imajuscule
{
    class Color : public ParamSet
    {
    public:
        Color();
        virtual ~Color();

        void getParams(float & alpha);
    private:
        enum
        {
            INDEX_ALPHA = 0,
        };
    };
}