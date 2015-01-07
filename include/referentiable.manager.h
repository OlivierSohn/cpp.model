#pragma once

#include <map>
#include <string>
#include <vector>

#include "visitable.h"

namespace imajuscule
{
    class Referentiable;
    typedef std::vector<Referentiable*> referentiables;
    class ReferentiableManager : public Visitable
    {
    public:
        ReferentiableManager();
        virtual ~ReferentiableManager();

        // guid is unique
        Referentiable * findByGuid(const std::string & guid);
        // session name is unique per-session
        Referentiable * findBySessionName(const std::string & sessionName);

        void ListReferentiablesByCreationDate(referentiables& vItems);

        PERSISTABLE_VISITOR_HEADER_IMPL

    protected:
        // pure virtual because the session names are unique "per object type"
        bool ComputeSessionName(Referentiable*);

        bool Register(Referentiable*, const std::string& sessionName);
        static void generateGuid(std::string & guid);

    private:
        // guid - referentiable
        typedef std::map<std::string, Referentiable*> guidsToRftbls;
        // session name - referentiable
        typedef std::map< std::string, Referentiable*> snsToRftbls;

        snsToRftbls m_snsToRftbls;
        guidsToRftbls m_guidsToRftbls;
    };
}