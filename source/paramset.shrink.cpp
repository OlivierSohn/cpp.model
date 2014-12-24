#include "paramset.shrink.h"
using namespace imajuscule;

// TODO lifecyle...
Shrink::Shrink() :
ParamSet("Shrink", paramsInSet{ new Param<float>("SHRINK", 0.5f ) })
{

}

Shrink::~Shrink()
{
}

void Shrink::getParams(float & shrink)
{
    shrink = (static_cast<Param<float>&> (getParam(Shrink::INDEX_SHRINK))).GetValue();
}
