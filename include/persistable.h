
#pragma once

#include "updatable.h"

namespace imajuscule
{
    enum PersistableEvent
    {
        VALUE_CHANGED = 0,
        OBJECT_DELETE
    };

    class Persistable : public Updatable
    {
    public:
        virtual ~Persistable();

        Observable<PersistableEvent, Persistable*> & observable();

    protected:
        Persistable();

    private:        
        Observable<PersistableEvent, Persistable*> m_observable;
    };
}
