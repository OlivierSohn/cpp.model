#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>

#include "persistable.h"
#include "observable.h"
#include "os.storage.keys.h"

#define LINK(type) RefLink<type>
#define ELINK(value) *this, value
#define ILINKVAL(name,val) name ( ELINK(val) )
#define ILINK(name) ILINKVAL(name,NULL)
#define CLINK(type,value) ILINKVAL(LINK(type), value)
#define LV_LINK(type,value) LINK(type) ( ELINK(value) )
namespace imajuscule
{
    class ReferentiableManagerBase;
    class ReferentiableCmdBase;
    class Command;
    class Referentiable;
    typedef std::vector<Referentiable*> refs;

    class Referentiable : public Persistable
    {
        friend class ReferentiableManagerBase;
    public:
        static Referentiable * instantiate(ReferentiableManagerBase * rm);
        static Referentiable * instantiate(ReferentiableManagerBase * rm, const std::string & hintName);
        void deinstantiate();
        enum Event
        {
            DEACTIVATE_LINKS,
            WILL_BE_DELETED,
            SOURCES_CHANGED,
            TARGETS_CHANGED
        };
        Observable<Event, Referentiable*> & observableReferentiable();

        const std::string & guid() const;
        const std::string & sessionName() const;
        std::string extendedName() const;
        const std::string & hintName() const;
        const std::string & creationDate() const;

        ReferentiableManagerBase * getManager() const;

        void Hide();
        bool isHidden();
        
        static bool ReadIndexForDiskGUID(const Storage::DirectoryPath & path, const std::string & guid, unsigned int &index, std::string & sHintName);
        
        void registerSource( Referentiable& source );
        void registerTarget( Referentiable& target );
        size_t countTargets();
        size_t countSources();
        void unRegisterTarget( Referentiable& target );
        void unRegisterSource( Referentiable& source );
        
        void traverseTargets(refs::iterator & begin, refs::iterator & end);
        void traverseSources(refs::iterator & begin, refs::iterator & end);
    protected:
        std::vector<Referentiable*> m_others;
        
        virtual ~Referentiable();

        virtual void Init() {};

        // this version of the contructor doesn't set creation Date 
        Referentiable();
        Referentiable(ReferentiableManagerBase * manager, const std::string & guid);
        Referentiable(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName);
        
        DECL_PERSIST(Referentiable, Persistable)
        
        class ReferentiableIndexLoad : public KeysLoad
        {
        public:
            ReferentiableIndexLoad( DirectoryPath, FileName);
            ~ReferentiableIndexLoad();
            
            bool found(unsigned int &index, std::string & nameHint);
            const std::string & dateCrea();
        
        protected:
            void LoadInt32ForKey(char key, int32_t i) override;
            void LoadStringForKey(char key, std::string & sVal) override;

        private:
            bool m_bFound;
            std::string m_hintName, m_dateOfCreation;
            unsigned int m_uiIndex;
        };
        
    private:
        ReferentiableManagerBase * m_manager;
        std::string m_guid; // persisted
        std::string m_hintName; // persisted
        std::string m_sessionName; // not persisted
        std::string m_dateOfCreation; // persisted
        refs m_targets, m_sources;
        Observable<Event, Referentiable*> * m_observableReferentiable;

        bool m_bHidden;
        bool m_bHasSessionName;
        virtual void setSessionName(const std::string & sn);

        virtual Referentiable * mainRefAttr() const;
    };
    
    template <class T>
    class RefLink
    {
    public:
        RefLink(Referentiable& source, T *target);
        RefLink(RefLink && r);
        ~RefLink();
        operator T*();
        operator const T*() const;
        RefLink & operator= (RefLink & other);
        RefLink & operator= (T * pointer);
        const T* operator->() const;
        T* operator->();
        const T& operator*() const;
        T& operator*();
        T * get();
        T * getConst() const;
        const T * get() const;

        bool operator < (RefLink & other);
        
    private:
        T* m_target;
        Referentiable & m_source;
        bool m_bActive;
        bool m_bTargetIsUp;

        std::vector<FunctionInfo<Referentiable::Event>> m_targetRegs;
        
        void set(T * target);
        RefLink();
        void deactivate();
        
        void RegisterTargetCb();
        void UnregisterTargetCb();
    };

}

#include "referentiable.link.hpp"

