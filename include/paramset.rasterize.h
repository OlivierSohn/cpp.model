#pragma once

#include "paramset.h"

namespace imajuscule
{
    class Rasterize : public ParamSet
    {
    public:
        Rasterize();
        virtual ~Rasterize();

        void getParams(float & gridSize, bool & bAsVoxels);
    private:
        enum
        {
            INDEX_GRIDSIZE = 0,
            INDEX_ASVOXELS = 1
        };
    };
}