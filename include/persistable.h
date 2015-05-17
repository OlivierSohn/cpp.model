
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
virtual void LoadStringArrayForKey(char key, std::vector<std::string> const &); \
virtual void LoadInt32ForKey(char key, int32_t); \
private: \
type & m_ ## type; \
};\
private:

// persistence of strings, stringarrays
#define IMPL_PERSIST3( type, supertype, implSave, ilString, ilStringArray, ilInt32 ) \
namespace imajuscule { \
template class RefLink<imajuscule::type> ; \
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
ilString \
default: \
supertype ## Load::LoadStringForKey(key, str); \
break; \
} \
} \
void type::type ## Load ::LoadStringArrayForKey(char key, std::vector<std::string> const & vs) { \
switch(key) \
{ \
ilStringArray \
default: \
supertype ## Load::LoadStringArrayForKey(key, vs); \
break; \
} \
} \
void type::type ## Load ::LoadInt32ForKey(char key, int32_t intVal) { \
switch(key) \
{ \
ilInt32 \
default: \
supertype ## Load::LoadInt32ForKey(key, intVal); \
break; \
} \
} \
void type::Load(Storage::DirectoryPath d, Storage::FileName f) \
{ \
type::type ## Load l( d, f, *this);  \
l.ReadAllKeys();\
} \
}

#define IMPL_PERSIST2( type, supertype, implSave, implLoad, implLoad2 ) IMPL_PERSIST3( type, supertype, implSave, implLoad, implLoad2, )

#define IMPL_PERSIST( type, supertype, implSave, implLoad ) IMPL_PERSIST2( type, supertype, implSave, implLoad, )

#define UGLY_SAVE_REF(ref) \
eResult res = ref->Save(); \
A(ILE_SUCCESS == res);

#define W_LNK( refExpr, key ) \
if(Referentiable * ref = refExpr) \
{       \
    WriteKeyData(key, ref->guid()); \
    UGLY_SAVE_REF(ref);\
}

#define W_LNK_ELT( ref, vs ) \
if_A(ref) \
{\
    vs.push_back(ref->guid());\
    UGLY_SAVE_REF(ref);\
}

#define W_LNKS( vec, key ) \
std::vector<std::string> vs; \
for(auto & it : vec) \
{ \
    Referentiable * ref = it; \
    W_LNK_ELT( ref, vs); \
} \
WriteKeyData(key, vs);

#define W_LNKS_P1( container, key ) \
std::vector<std::string> vs; \
for(auto & it : container) \
{\
    Referentiable * ref = it.first; \
    W_LNK_ELT( ref, vs); \
}\
WriteKeyData(key, vs);

#define R_LNKS_OP( Op, type, key ) \
case key: \
    for(auto const & guid : vs)\
        Op( static_cast<type*>(Referentiables::fromGUID(Storage::curDir(), guid)) );\
break;

#define R_LNK_OP( Op, type, key ) \
case key: \
Op( static_cast<type*>(Referentiables::fromGUID(Storage::curDir(), str)) );\
break;

namespace imajuscule
{
    enum PersistableEvent
    {
        OBJECT_DEFINITION_CHANGED = 0 // TODO this event should be in Referentiable
    };

    class Persistable : public Updatable
    {
    public:
        virtual ~Persistable();

        Observable<PersistableEvent, Persistable*> & observable();

        virtual void Load(Storage::DirectoryPath d, Storage::FileName f){A(0);}
        virtual eResult Save() {A(0); return ILE_ERROR;}
        
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
