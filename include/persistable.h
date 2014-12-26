
#pragma once

#include "observable.h"
#include "visitor.persistable.h"

namespace imajuscule
{
    class Persistable : public Observable
    {
    public:
        virtual ~Persistable();

        PERSISTABLE_VISITOR_PURE_VIRTUAL

    protected:
        Persistable();
    };
}
