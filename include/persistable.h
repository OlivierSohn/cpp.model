
#pragma once

#include "updatable.h"

namespace imajuscule
{
    class Persistable : public Updatable
    {
    public:
        virtual ~Persistable();

    protected:
        Persistable();
    };
}
