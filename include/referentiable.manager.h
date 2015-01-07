#pragma once

#include <map>
#include <string>
#include <vector>

namespace imajuscule
{
    class Referentiable;
    class ReferentiableManager
    {
    public:

        typedef std::vector<Referentiable*> referentiables;

        ReferentiableManager();
        virtual ~ReferentiableManager();

        // guid is unique
        Referentiable * findByGuid(const std::string & guid);
        // session name is unique per-session
        Referentiable * findBySessionName(const std::string & sessionName);

        void ListReferentiablesByCreationDate(referentiables& vItems);

        // pure virtual because the session names are unique "per object type"
        virtual bool ComputeSessionName(Referentiable*) = 0;
    protected:

        bool Register(Referentiable*, const std::string& sessionName);
    private:
        // guid - referentiable
        typedef std::map<std::string, Referentiable*> guidsToRftbls;
        // session name - referentiable
        typedef std::map< std::string, Referentiable*> snsToRftbls;

        snsToRftbls m_snsToRftbls;
        guidsToRftbls m_guidsToRftbls;

        static void generateGuid(std::string & guid);
    };
}