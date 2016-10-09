#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "ref_unique_ptr.h"
#include "persistable.h"
#include "observable.h"
#include "os.storage.keys.h"

namespace imajuscule
{
    class ReferentiableManagerBase;
    
    template <typename>
    class ReferentiableManager; // so that all referentiables can friend their manager

    class ReferentiableCmdBase;
    class Command;
    class Referentiable;
    typedef std::vector<Referentiable*> refs;

    class Referentiable : public Persistable
    {
        friend class ReferentiableManagerBase;
    public:
        Referentiable(Referentiable&& other)
        {
            m_guid = std::move(other.m_guid);
            m_hintName = std::move(other.m_hintName);
            m_sessionName = std::move(other.m_sessionName);
            m_dateOfCreation = std::move(other.m_dateOfCreation);
            m_targets = std::move(other.m_targets);
            m_sources = std::move(other.m_sources);
            m_observableReferentiable = std::move(other.m_observableReferentiable);
            m_bHidden = std::move(other.m_bHidden);
            m_bHasSessionName = std::move(other.m_bHasSessionName);
        }
        
        Referentiable& operator=(Referentiable&& other)
        {
            if (this != &other)
            {
                m_guid = std::move(other.m_guid);
                m_hintName = std::move(other.m_hintName);
                m_sessionName = std::move(other.m_sessionName);
                m_dateOfCreation = std::move(other.m_dateOfCreation);
                m_targets = std::move(other.m_targets);
                m_sources = std::move(other.m_sources);
                m_observableReferentiable = std::move(other.m_observableReferentiable);
                m_bHidden = std::move(other.m_bHidden);
                m_bHasSessionName = std::move(other.m_bHasSessionName);
            }
            return *this;
        }
        
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
        
        static bool ReadIndexForDiskGUID(const DirectoryPath & path, const std::string & guid, unsigned int &index, std::string & sHintName);
        
        void registerSource( Referentiable& source );
        void registerTarget( Referentiable& target );
        size_t countTargets();
        size_t countSources();
        void unRegisterTarget( Referentiable& target );
        void unRegisterSource( Referentiable& source );
        
        void traverseTargets(refs::iterator & begin, refs::iterator & end);
        void traverseSources(refs::iterator & begin, refs::iterator & end);
    protected:
        
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

}
#define SET_ref_unique(type, name, methodPostFix) \
void set##methodPostFix(ref_unique_ptr<type> p) { \
    if(p == name) {\
        return;\
    }\
    removeSpec(name.get());\
    name.swap(p);\
    addSpec(name.get());\
}


