#pragma once

#include "visitor.h"
#include "observable.h"

#define VISITOR_PURE_VIRTUAL virtual void accept(Visitor &) = 0;

#define VISITOR_HEADER_IMPL void accept(Visitor & vtor) override\
        {\
        vtor.Visit(this);\
        }


namespace imajuscule
{    
    class Visitable
    {
    public:
        enum HierarchyEvent
        {
            ADD_CHILD,
            REMOVE_CHILD
        };
        enum Event
        {
            VISITABLE_DELETE,
        };
        virtual ~Visitable();

        Observable<Event, Visitable&> &  observableVisitable();
        Observable<HierarchyEvent, Visitable&, Visitable&> &  observableVisitableH();

        VISITOR_PURE_VIRTUAL
    protected:
        Visitable();

    private:
        Observable<Event, Visitable&> * m_observableVisitable;
        Observable<HierarchyEvent, Visitable&, Visitable&> * m_observableVisitableH;
    };
}
