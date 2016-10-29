
#pragma once

#include "updatable.h"
#include "observable.h"
#include "os.storage.keys.h"
#include "os.storage.h"

#define DECL_NO_PERSIST         eResult Save(const Directory) override {return ILE_SUCCESS;}

#define DECL_PERSIST_CLASSES( type, supertype ) \
class type ## Persist : public supertype ## Persist { \
public: \
    type ## Persist(DirectoryPath, FileName, type & r); \
    eResult doSave() override; \
private: \
    type & m_ ## type; \
}; \
class type ## Load : public supertype ## Load { \
public: \
    type ## Load( DirectoryPath, FileName, type&); \
protected: \
    void LoadStringForKey(char key, std::string & str) override; \
    void LoadStringArrayForKey(char key, std::vector<std::string> const &) override; \
    void LoadInt32ForKey(char key, int32_t) override; \
    void onLoadFinished() override; \
private: \
    type & m_ ## type; \
};

#define DECL_PERSIST( type, supertype ) \
public: \
void Load(DirectoryPath d, Storage::FileName f) override; \
eResult Save(const DirectoryPath &) override; \
protected: \
DECL_PERSIST_CLASSES( type, supertype ) \
private:

// persistence of strings, stringarrays
#define IMPL_PERSIST_CLASSES( type, supertype, implSave, ilString, ilStringArray, ilInt32 ) \
namespace imajuscule { \
type::type ## Persist :: type ## Persist(DirectoryPath d, FileName f, type & r) : supertype ## Persist(d, f, r), m_ ## type(r) {} \
eResult type::type ## Persist::doSave() { \
implSave \
return supertype ## Persist::doSave(); \
} \
type::type ## Load :: type ## Load(DirectoryPath d, FileName f, type & r) : supertype ## Load(d, f, r), m_ ## type(r) {} \
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
void type::type ## Load ::onLoadFinished() { \
m_ ## type.onLoaded(); \
} \
}

#define IMPL_PERSIST3( type, supertype, implSave, ilString, ilStringArray, ilInt32 ) IMPL_PERSIST_CLASSES( type, supertype, implSave, ilString, ilStringArray, ilInt32) \
namespace imajuscule { \
eResult type::Save(const DirectoryPath & p) \
{ \
type::type ## Persist l( p, guid(), *this);  \
eResult res = l.Save();\
return res;\
} \
void type::Load(DirectoryPath d, Storage::FileName f) \
{ \
type::type ## Load l( d, f, *this);  \
l.ReadAllKeys();\
} \
}

#define IMPL_PERSIST2( type, supertype, implSave, implLoad, implLoad2 ) IMPL_PERSIST3( type, supertype, implSave, implLoad, implLoad2, )

#define IMPL_PERSIST( type, supertype, implSave, implLoad ) IMPL_PERSIST2( type, supertype, implSave, implLoad, )

#define UGLY_SAVE_REF(ref) \
eResult res = ref->Save(directory()); \
A(ILE_SUCCESS == res || ILE_RECURSIVITY == res);

#define W_LNK_SOFT( refExpr, key ) \
{\
Referentiable * ref = static_cast<Referentiable*>(refExpr);\
if(ref) \
{       \
WriteKeyData(key, ref->guid()); \
UGLY_SAVE_REF(ref);\
}\
}


#define W_PTR( refExpr, key ) \
{\
A(refExpr); \
W_LNK_SOFT(refExpr, key); \
}

#define W_LNK( refExpr, key ) \
{\
W_PTR(refExpr.get(), key); \
}


#define W_LNK_ELT( ref, vs ) \
if_A(ref) \
{\
    vs.push_back(ref->guid());\
    UGLY_SAVE_REF(ref);\
}

#define W_LNKS( vec, key ) \
{ \
std::vector<std::string> vs; \
for(auto const & link : vec) \
{ \
Referentiable * ref = &*link; \
W_LNK_ELT( ref, vs); \
} \
WriteKeyData(key, vs);  \
}

#define W_LNKS_P1( container, key ) \
{ \
std::vector<std::string> vs; \
for(auto & it : container) \
{\
    Referentiable * ref = it.first.get(); \
    W_LNK_ELT( ref, vs); \
}\
WriteKeyData(key, vs);  \
}

#define R_LNKS_OP( Op, type, key ) \
case key: \
    for(auto const & guid : vs)\
        Op( static_cast<type*>(Referentiables::fromGUID(directory(), guid)) );\
break;

#define R_UNIQUE_LNKS_OP( Op, type, key ) \
case key: \
    for(auto const & guid : vs)\
        Op( ref_unique_ptr<type>(static_cast<type*>(Referentiables::fromGUID(directory(), guid))) );\
break;

#define L_LNKS( key ) \
case key: \
    for( auto const & guid : vs ) { Referentiables::fromGUID(directory(), guid); }\
break;

#define R_LNK_OP( Op, type, key ) \
case key: \
Op( static_cast<type*>(Referentiables::fromGUID(directory(), str)) );\
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

        virtual void Load(DirectoryPath d, Storage::FileName f){A(0);}
        virtual eResult Save( DirectoryPath const & ) {A(0); return ILE_ERROR;}
        
        // overload by calling supertype implementation first
        virtual void onLoaded() {};
        
    protected:
        Persistable();

        FunctionInfo<PersistableEvent> addSpecAndForwardNotifications(Persistable * upd);
        void removeSpecAndUnforward(Persistable * upd, const FunctionInfo<PersistableEvent> & reg);

        // these are not defined through macros
        class PersistablePersist : public KeysPersist
        {
        public:
            PersistablePersist(DirectoryPath d, FileName f, Persistable & p);
            
            eResult doSave() override;
            
        private:
            Persistable & m_persistable;
        };
        virtual eResult Save(PersistablePersist&);
        
        class PersistableLoad : public KeysLoad
        {
        public:
            PersistableLoad(DirectoryPath d, FileName f, Persistable & p);
        private:
            Persistable & m_persistable;
        };
    private:
        Observable<PersistableEvent, Persistable*> * m_observable;
    };
    
}
