#include "persistable.h"

using namespace imajuscule;

Persistable::~Persistable()
{
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

eResult Persistable::Save(PersistablePersist&pp)
{
    eResult ret = ILE_NOT_IMPLEMENTED;
    A(0);
    return ret;
}

Persistable::PersistablePersist::PersistablePersist(DirectoryPath d, FileName f, Persistable & p):
KeysPersist(d, f)
, m_persistable(p)
{
}

Persistable::PersistablePersist::~PersistablePersist()
{
}
    
eResult Persistable::PersistablePersist::Save()
{
    eResult ret = ILE_SUCCESS;
    return ret;
}

Persistable::PersistableLoad::PersistableLoad(DirectoryPath d, FileName f) :
 KeysLoad(d,f)
{
}
Persistable::PersistableLoad::~PersistableLoad()
{
    
}
