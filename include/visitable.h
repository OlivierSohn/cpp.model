#pragma once

#include "visitor.h"

#define PERSISTABLE_VISITOR_PURE_VIRTUAL virtual void accept(Visitor &) = 0;

#define PERSISTABLE_VISITOR_HEADER_IMPL virtual void accept(Visitor & vtor) override\
        {\
        vtor.Visit(this);\
        }


namespace imajuscule
{    
    class Visitable
    {
    public:
        virtual ~Visitable(){}

        PERSISTABLE_VISITOR_PURE_VIRTUAL
    protected:
        Visitable() {}
    };
}
