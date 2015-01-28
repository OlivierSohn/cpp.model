#include "persistable.h"

using namespace imajuscule;

Persistable::~Persistable()
{
    m_observable->Notify(PersistableEvent::OBJECT_DELETE, this);
    // perform delayed deletion of observable because we might be inside a Notify call
    m_observable->deinstantiate();
}

Persistable::Persistable():
Updatable(),
m_observable(Observable<PersistableEvent, Persistable*>::instantiate())
{
}

Observable<PersistableEvent, Persistable*> & Persistable::observable()
{
    return *m_observable;
}


const FunctionInfo<PersistableEvent> Persistable::addSpecAndForwardNotifications(Persistable * upd)
{
    addSpec(upd);
    return upd->observable().Register(OBJECT_DEFINITION_CHANGED, [this](Persistable*){ observable().Notify(OBJECT_DEFINITION_CHANGED, this); });
}

void Persistable::removeSpecAndDelete(Persistable * upd)
{
    removeSpec(upd);
    delete upd;
}
void Persistable::removeSpecAndUnforward(Persistable * upd, const FunctionInfo<PersistableEvent> & reg)
{
    removeSpec(upd);
    upd->observable().Remove(reg);
}
