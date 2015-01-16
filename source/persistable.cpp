#include "persistable.h"

using namespace imajuscule;

Persistable::~Persistable()
{
    m_observable.Notify(PersistableEvent::OBJECT_DELETE, this);
}

Persistable::Persistable():
Updatable()
{
}

Observable<PersistableEvent, Persistable*> & Persistable::observable()
{
    return m_observable;
}
