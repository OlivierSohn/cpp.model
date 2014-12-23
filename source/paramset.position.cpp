#include "paramset.position.h"
using namespace imajuscule;

Position::Position() :
ParamSet("Position", paramsInSet{
    Param<float>("tx", 0.f),
    Param<float>("ty", 0.f),
    Param<float>("tz", 0.f),
    Param<float>("rx", 0.f),
    Param<float>("ry", 0.f),
    Param<float>("rz", 0.f) })
{

}

Position::~Position()
{
}
