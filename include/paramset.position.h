#pragma once

#include "paramset.h"

namespace imajuscule
{
    class Position : public ParamSet
    {
    public:
        Position();
        virtual ~Position();

        void getTranslation(float trans[3]);
        void getRotation(float rot[3]);
    private:
        enum
        {
            INDEX_TX = 0,
            INDEX_TY = 1,
            INDEX_TZ = 2,
            INDEX_RX = 3,
            INDEX_RY = 4,
            INDEX_RZ = 5
        };
    };
}