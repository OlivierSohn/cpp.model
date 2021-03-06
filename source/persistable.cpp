
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


FunctionInfo<PersistableEvent> Persistable::addSpecAndForwardNotifications(Persistable * upd)
{
    addSpec(upd);
    return upd->observable().Register(PersistableEvent::OBJECT_DEFINITION_CHANGED, [this](Persistable*){ observable().Notify(PersistableEvent::OBJECT_DEFINITION_CHANGED, this); });
}

void Persistable::removeSpecAndUnforward(Persistable * upd, const FunctionInfo<PersistableEvent> & reg)
{
    if(removeSpec(upd)) {
        upd->observable().Remove(reg);
    }
}

eResult Persistable::Save(PersistablePersist&pp)
{
    eResult ret = ILE_NOT_IMPLEMENTED;
    Assert(0);
    return ret;
}

Persistable::PersistablePersist::PersistablePersist(DirectoryPath d, FileName f, Persistable & p):
KeysPersist(d, f)
, m_persistable(p)
{
}

    
eResult Persistable::PersistablePersist::doSave()
{
    eResult ret = ILE_SUCCESS;
    return ret;
}

Persistable::PersistableLoad::PersistableLoad(DirectoryPath d, FileName f, Persistable & p) :
 KeysLoad(d,f)
, m_persistable(p)
{
}
