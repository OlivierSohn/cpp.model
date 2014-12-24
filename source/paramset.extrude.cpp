#include "paramset.extrude.h"
using namespace imajuscule;

Extrude::Extrude() :
ParamSet("Extrude", paramsInSet{ new Param<int>("AXIS", 0), new Param<int>("LENGTH", 10) })
{

}

Extrude::~Extrude()
{
}


void Extrude::getParams(int & axis, int & length)
{
    axis = static_cast<Param<int>&> (getParam(Extrude::INDEX_AXISINDEX)).GetValue();
    length = static_cast<Param<int>&> (getParam(Extrude::INDEX_LENGTH)).GetValue();
}
