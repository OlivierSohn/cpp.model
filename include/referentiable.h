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
        const std::string & hintName() const;
        const std::string & creationDate() const;

        ReferentiableManagerBase * getManager();

        void Hide();
        bool isHidden();

    protected:
        virtual ~Referentiable();

        virtual void Init() {};

        // this version of the contructor doesn't set creation Date 
        Referentiable();
        Referentiable(ReferentiableManagerBase * manager, const std::string & guid);
        Referentiable(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName);

        class ReferentiablePersist : public KeysPersist
        {
        public:
            ReferentiablePersist(){}
            virtual ~ReferentiablePersist(){}

            virtual eResult Save();

        protected:
            virtual Referentiable * ref() = 0;
        };

        class ReferentiableLoad : public KeysLoad
        {
        public:
            ReferentiableLoad(){}
            virtual ~ReferentiableLoad(){}

        protected:
            virtual Referentiable * ref() = 0;
            virtual void LoadStringForKey(char key, std::string & str);
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
    };
}

#include "referentiable.manager.h"