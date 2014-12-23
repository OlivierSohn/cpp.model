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


void Position::getTranslation(float trans[3])
{
    trans[0] = static_cast<Param<float>&> (getParam(Position::INDEX_TX)).GetValue();
    trans[1] = static_cast<Param<float>&> (getParam(Position::INDEX_TY)).GetValue();
    trans[2] = static_cast<Param<float>&> (getParam(Position::INDEX_TZ)).GetValue();
}

void Position::getRotation(float rot[3])
{
    rot[0] = static_cast<Param<float>&> (getParam(Position::INDEX_RX)).GetValue();
    rot[1] = static_cast<Param<float>&> (getParam(Position::INDEX_RY)).GetValue();
    rot[2] = static_cast<Param<float>&> (getParam(Position::INDEX_RZ)).GetValue();
}