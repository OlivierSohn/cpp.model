#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>

#include "persistable.h"

#include "os.storage.keys.h"

namespace imajuscule
{
    class ReferentiableManagerBase;
    class Referentiable : public Persistable
    {
        friend class ReferentiableManagerBase;
    public:
        const std::string & guid();
        const std::string & sessionName();
        const std::string & hintName();
        const std::string & creationDate();

    protected:
        virtual ~Referentiable();

        virtual void Init() {};

        // this version of the contructor doesn't set creation Date 
        Referentiable();
        Referentiable(ReferentiableManagerBase * manager, const std::string & guid);
        Referentiable(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName);

        ReferentiableManagerBase * getManager();

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

        bool m_bHasSessionName;
        virtual void setSessionName(const std::string & sn);
    };
}