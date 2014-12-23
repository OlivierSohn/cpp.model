#include"positionable.body.h"
#include "positionable.joint.h"

using namespace imajuscule;

Body::Body(Joint * parent) :
Positionable(parent)
{
}

Body::~Body()
{
}

void Body::doUpdate()
{
    // NOTE is it necessary for body to be updatable? his operations already are...

    // at this point, all specs have been updated
    // and at least one observed has changed
}

