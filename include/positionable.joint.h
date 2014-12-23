#pragma once
#include "positionable.h"

#include <vector>

namespace imajuscule
{
    class Joint : public Positionable
    {
    public:
        typedef std::vector < Joint* > joints;

        // ownership of children transferred to this
        Joint(Joint * parent, joints & children);
        virtual ~Joint();

        void doUpdate() override;

        PERSISTABLE_VISITOR_HEADER_IMPL

    private:
        joints m_childJoints;
    };
}