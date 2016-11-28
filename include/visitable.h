#pragma once

#include "observable.h"


#define VISITOR_HEADER_IMPL void accept(Visitor & vtor) override {\
    vtor.Visit(this);\
}


namespace imajuscule
{
    struct NonCopyable {
        NonCopyable() =default;
        NonCopyable(const NonCopyable &) = delete;
        NonCopyable & operator=(const NonCopyable&) = delete;
    };
    
    struct Object : public NonCopyable {
        virtual ~Object() = default;
    };

    class Visitor;
    class Visitable : public Object
    {
    public:
        enum HierarchyEvent {
            ADD_CHILD,
            REMOVE_CHILD
        };
        enum Event {
            VISITABLE_DELETE,
        };
        
        ~Visitable();

        Observable<Event, Visitable&> &  observableVisitable();
        Observable<HierarchyEvent, Visitable&, Visitable&> &  observableVisitableH();

        virtual void accept(Visitor &) = 0;
    protected:
        Visitable();

    private:
        Observable<Event, Visitable&> * m_observableVisitable;
        Observable<HierarchyEvent, Visitable&, Visitable&> * m_observableVisitableH;
    };
}
