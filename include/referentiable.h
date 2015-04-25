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

    protected:
        virtual ~Referentiable();

        virtual void Init() {};

        // this version of the contructor doesn't set creation Date 
        Referentiable();
        Referentiable(ReferentiableManagerBase * manager, const std::string & guid);
        Referentiable(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName);

        class ReferentiablePersist : public PersistablePersist
        {
        public:
            ReferentiablePersist(DirectoryPath, FileName, Referentiable & r);
            virtual ~ReferentiablePersist();

            virtual eResult Save();

        protected:
            Referentiable & m_ref;
        };

        class ReferentiableLoad : public PersistableLoad
        {
        public:
            ReferentiableLoad( DirectoryPath, FileName, Referentiable&);
            virtual ~ReferentiableLoad();

        protected:
            virtual void LoadStringForKey(char key, std::string & str);
            
        private:
            Referentiable & m_ref;
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
