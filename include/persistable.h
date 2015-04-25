
#pragma once

#include "updatable.h"
#include "observable.h"
#include "os.storage.keys.h"

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

        class PersistablePersist : public KeysPersist
        {
        public:
            PersistablePersist(DirectoryPath d, FileName f, Persistable & p);
            virtual ~PersistablePersist();
            
            eResult Save();
            
        private:
            Persistable & m_persistable;
        };
        virtual eResult Save(PersistablePersist&);
        
        class PersistableLoad : public KeysLoad
        {
        public:
            PersistableLoad(DirectoryPath d, FileName f);
            virtual ~PersistableLoad();
            
            eResult Load();
        };
    private:
        Observable<PersistableEvent, Persistable*> * m_observable;
        
        
    };
}
