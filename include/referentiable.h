#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>

#include "persistable.h"

#include "os.storage.keys.h"

namespace imajuscule
{
    class ReferentiableManager;
    class Referentiable : public Persistable
    {
        friend class ReferentiableManager;
    public:
        const std::string & guid();
        const std::string & sessionName();
        const std::string & hintName();
        const std::string & creationDate();

//        typedef std::vector<Persistable*> referencers;
//        void traverseReferencers(referencers::iterator & begin, referencers::iterator & end);
    protected:
        virtual ~Referentiable();

        // this version of the contructor doesn't set creation Date 
        Referentiable(ReferentiableManager * manager, const std::string & guid);
        Referentiable(ReferentiableManager * manager, const std::string & guid, const std::string & hintName);

        ReferentiableManager * getManager();

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
        ReferentiableManager * m_manager;
        std::string m_guid; // persisted
        std::string m_hintName; // persisted
        std::string m_sessionName; // not persisted
        std::string m_dateOfCreation; // persisted

        bool m_bHasSessionName;
        void setSessionName(const std::string & sn);

//        referencers m_referencers;
    };
}