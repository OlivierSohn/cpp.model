#pragma once

#include <map>
#include <string>
#include <vector>

#include "visitable.h"
#include "observable.h"

namespace imajuscule
{
    class Referentiable;
    typedef std::vector<Referentiable*> referentiables;
    class ReferentiableManagerBase : public Visitable
    {
    public:
        enum class Event
        {
            RFTBL_ADD, // a referentiable was added to the list of referentiables managed by the manager
            RFTBL_REMOVE, // a referentiable was removed from the list of referentiables managed by the manager
            MANAGER_DELETE// the manager is being deleted
        };
        ReferentiableManagerBase();
        virtual ~ReferentiableManagerBase();

        // guid is unique
        Referentiable * findByGuid(const std::string & guid);
        // session name is unique per-session
        Referentiable * findBySessionName(const std::string & sessionName);

        void ListReferentiablesByCreationDate(referentiables& vItems);

        Observable<Event, Referentiable*> & observable();

        void Remove(Referentiable*);

        PERSISTABLE_VISITOR_HEADER_IMPL

    protected:
        // pure virtual because the session names are unique "per object type"
        bool ComputeSessionName(Referentiable*);

        bool RegisterWithSessionName(Referentiable*, const std::string& sessionName);
        static void generateGuid(std::string & guid);

    private:
        // guid - referentiable
        typedef std::map<std::string, Referentiable*> guidsToRftbls;
        // session name - referentiable
        typedef std::map<std::string, Referentiable*> snsToRftbls;

        snsToRftbls m_snsToRftbls;
        guidsToRftbls m_guidsToRftbls;

        Observable<Event, Referentiable*> * m_observable;
    };

    template <class T>
    class ReferentiableManager : public ReferentiableManagerBase
    {
    public:
        static ReferentiableManager * getInstance();

        T* newReferentiable(const std::string & nameHint, const std::vector<std::string> & guids);

    private:
        static ReferentiableManager * g_pAnimationManager;

        ReferentiableManager();
        virtual ~ReferentiableManager();
    };
}