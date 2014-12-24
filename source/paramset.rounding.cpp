#include "paramset.rounding.h"
using namespace imajuscule;

Rounding::Rounding() :
ParamSet("Rounding", paramsInSet{ new Param<float>("RR", 0.f), new Param<float>("RS", 0.f) })
{

}

Rounding::~Rounding()
{
}


void Rounding::getRoundings(float & roundingRatio, float & roundingStrength)
{
    roundingRatio = static_cast<Param<float>&> (getParam(Rounding::INDEX_RR)).GetValue();
    roundingStrength = static_cast<Param<float>&> (getParam(Rounding::INDEX_RS)).GetValue();
}
