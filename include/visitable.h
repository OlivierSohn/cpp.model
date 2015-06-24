#pragma once

#include "visitor.h"
#include "observable.h"

#define PERSISTABLE_VISITOR_PURE_VIRTUAL virtual void accept(Visitor &) = 0;

#define PERSISTABLE_VISITOR_HEADER_IMPL void accept(Visitor & vtor) override\
        {\
        vtor.Visit(this);\
        }


namespace imajuscule
{    
    class Visitable
    {
    public:
        enum Event
        {
            VISITABLE_DELETE
        };
        virtual ~Visitable();

        Observable<Event, Visitable&> &  observableVisitable();

        PERSISTABLE_VISITOR_PURE_VIRTUAL
    protected:
        Visitable();

    private:
        Observable<Event, Visitable&> * m_observableVisitable;
    };
}
