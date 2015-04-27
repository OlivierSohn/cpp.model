
#pragma once

#include "updatable.h"
#include "observable.h"
#include "os.storage.keys.h"
#include "os.storage.h"

#define DECL_PERSIST( type, supertype ) \
public: \
void Load(Storage::DirectoryPath d, Storage::FileName f) override; \
eResult Save() override; \
protected: \
class type ## Persist : public supertype ## Persist { \
public: \
type ## Persist(DirectoryPath, FileName, type & r); \
virtual ~type ## Persist(); \
virtual eResult doSave() override; \
private: \
type & m_ ## type; \
}; \
class type ## Load : public supertype ## Load { \
public: \
type ## Load( DirectoryPath, FileName, type&); \
virtual ~type ## Load(); \
protected: \
virtual void LoadStringForKey(char key, std::string & str); \
private: \
type & m_ ## type; \
};\
private:

#define IMPL_PERSIST( type, supertype, implSave, implLoad ) \
type::type ## Persist :: type ## Persist(DirectoryPath d, FileName f, type & r) : supertype ## Persist(d, f, r), m_ ## type(r) {} \
type::type ## Persist ::~ type ## Persist () {} \
eResult type::type ## Persist::doSave() { \
implSave \
return supertype ## Persist::doSave(); \
} \
eResult type::Save() \
{ \
type::type ## Persist l( Storage::curDir(), guid(), *this);  \
eResult res = l.Save();\
return res;\
} \
type::type ## Load :: type ## Load(DirectoryPath d, FileName f, type & r) : supertype ## Load(d, f, r), m_ ## type(r) {} \
type::type ## Load ::~ type ## Load() {} \
void type::type ## Load ::LoadStringForKey(char key, std::string & str) { \
switch(key) \
{ \
implLoad \
default: \
supertype ## Load::LoadStringForKey(key, str); \
break; \
} \
} \
void type::Load(Storage::DirectoryPath d, Storage::FileName f) \
{ \
type::type ## Load l( d, f, *this);  \
l.ReadAllKeys();\
}


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

        virtual void Load(Storage::DirectoryPath d, Storage::FileName f){A(0);}
        virtual eResult Save(){A(0); return ILE_ERROR;}
        
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
            
            eResult doSave() override;
            
        private:
            Persistable & m_persistable;
        };
        virtual eResult Save(PersistablePersist&);
        
        class PersistableLoad : public KeysLoad
        {
        public:
            PersistableLoad(DirectoryPath d, FileName f, Persistable & p);
            virtual ~PersistableLoad();
        private:
            Persistable & m_persistable;
        };
    private:
        Observable<PersistableEvent, Persistable*> * m_observable;
        
        
    };
}
