#include "paramset.shrink.h"
using namespace imajuscule;

Shrink::Shrink() :
ParamSet("Shrink", paramsInSet{ Param<float>("ALPHA", 0.5f), Param<float>("RS", 0.f) })
{

}

Shrink::~Shrink()
{
}

void Shrink::getParams(float & shrink)
{
    shrink = static_cast<Param<float>&> (getParam(Shrink::INDEX_SHRINK)).GetValue();
}
