#include "paramset.rasterize.h"
using namespace imajuscule;

Rasterize::Rasterize() :
ParamSet("Rasterize", paramsInSet{ Param<float>("GridSize", 0.f), Param<bool>("As Voxels", false) })
{

}

Rasterize::~Rasterize()
{
}


void Rasterize::getParams(float & gridSize, bool & bAsVoxels)
{
    gridSize = static_cast<Param<float>&> (getParam(Rasterize::INDEX_GRIDSIZE)).GetValue();
    bAsVoxels = static_cast<Param<bool>&> (getParam(Rasterize::INDEX_ASVOXELS)).GetValue();
}
