#include"positionable.joint.h"

using namespace imajuscule;

Joint::Joint(Joint * parent, joints & children) :
Positionable(parent)
{
    m_childJoints.swap(children);
}

Joint::~Joint()
{
    joints::iterator it = m_childJoints.begin();
    joints::iterator end = m_childJoints.end();
    for (; it != end; ++it)
    {
        delete *it;
    }
}

void Joint::doUpdate()
{
    // should Joint be updatable?
}
