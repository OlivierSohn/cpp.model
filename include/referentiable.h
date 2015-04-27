#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>

#include "persistable.h"
#include "observable.h"
#include "os.storage.keys.h"

namespace imajuscule
{
    class ReferentiableManagerBase;
    class ReferentiableCmdBase;
    class Command;
    class Referentiable : public Persistable
    {
        friend class ReferentiableManagerBase;
    public:
        static Referentiable * instantiate(ReferentiableManagerBase * rm);
        static Referentiable * instantiate(ReferentiableManagerBase * rm, const std::string & hintName);
        void deinstantiate();
        enum Event
        {
            WILL_BE_DELETED
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
        
        static bool ReadIndexForDiskGUID(const std::string & guid, unsigned int &index, std::string & sHintName);
        
    protected:
        virtual ~Referentiable();

        virtual void Init() {};

        // this version of the contructor doesn't set creation Date 
        Referentiable();
        Referentiable(ReferentiableManagerBase * manager, const std::string & guid);
        Referentiable(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName);

        DECL_PERSIST(Referentiable, Persistable);
        
        class ReferentiableIndexLoad : public KeysLoad
        {
        public:
            ReferentiableIndexLoad( DirectoryPath, FileName);
            ~ReferentiableIndexLoad();
            
            bool found(unsigned int &index, std::string & nameHint);
        
        protected:
            void LoadInt32ForKey(char key, int32_t i) override;
            void LoadStringForKey(char key, std::string & sVal) override;

        private:
            bool m_bFound;
            std::string m_hintName;
            unsigned int m_uiIndex;
        };

    private:
        ReferentiableManagerBase * m_manager;
        std::string m_guid; // persisted
        std::string m_hintName; // persisted
        std::string m_sessionName; // not persisted
        std::string m_dateOfCreation; // persisted
        Observable<Event, Referentiable*> * m_observableReferentiable;

        bool m_bHidden;
        bool m_bHasSessionName;
        virtual void setSessionName(const std::string & sn);

        virtual Referentiable * mainRefAttr() const;
    };
}
