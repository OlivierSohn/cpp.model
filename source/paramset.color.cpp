#include "paramset.color.h"
using namespace imajuscule;

Color::Color() :
ParamSet("Color", paramsInSet{ new Param<float>("ALPHA", 0.5f), new Param<float>("RS", 0.f) })
{

}

Color::~Color()
{
}


void Color::getParams(float & alpha)
{
    alpha = static_cast<Param<float>&> (getParam(Color::INDEX_ALPHA)).GetValue();
}
