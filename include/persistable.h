
#pragma once

#include "updatable.h"
#include "observable.h"

namespace imajuscule
{
    enum PersistableEvent
    {
        OBJECT_DEFINITION_CHANGED = 0
    };

    class Persistable : public Updatable
    {
    public:
        virtual ~Persistable();

        Observable<PersistableEvent, Persistable*> & observable();

    protected:
        Persistable();

        const FunctionInfo<PersistableEvent> addSpecAndForwardNotifications(Persistable * upd);
        void removeSpecAndUnforward(Persistable * upd, const FunctionInfo<PersistableEvent> & reg);
        void removeSpecAndDelete(Persistable * upd);

    private:        
        Observable<PersistableEvent, Persistable*> * m_observable;
    };
}
