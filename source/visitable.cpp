#include "visitable.h"
#include "os.log.h"

using namespace imajuscule;

Visitable::Visitable():
m_observableVisitableH(Observable<HierarchyEvent, Visitable&, Visitable& >::instantiate())
,m_observableVisitable(Observable<Event, Visitable& >::instantiate())
{
}

Visitable::~Visitable()
{
    m_observableVisitable->Notify(Event::VISITABLE_DELETE, *this);
    m_observableVisitable->deinstantiate();
    m_observableVisitableH->deinstantiate();
}

auto Visitable::observableVisitable() -> Observable<Event, Visitable& > &
{
    return *m_observableVisitable;
}

auto Visitable::observableVisitableH() -> Observable<HierarchyEvent, Visitable&, Visitable& > &
{
    return *m_observableVisitableH;
}
