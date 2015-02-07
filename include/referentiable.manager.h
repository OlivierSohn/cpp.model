#pragma once

#include <map>
#include <string>
#include <vector>

#include "visitable.h"
#include "observable.h"
#include "command.h"

namespace imajuscule
{
    class ReferentiableNewCmdBase;

    class Referentiable;
    typedef std::vector<Referentiable*> referentiables;
    class ReferentiableManagerBase : public Visitable
    {
        friend class ReferentiableNewCmdBase;
    public:
        enum class Event
        {
            RFTBL_ADD, // a referentiable was added to the list of referentiables managed by the manager
            RFTBL_REMOVE, // a referentiable was removed from the list of referentiables managed by the manager
            MANAGER_DELETE// the manager is being deleted
        };
        ReferentiableManagerBase();
        virtual ~ReferentiableManagerBase();

        virtual std::string defaultNameHint() = 0;
        Referentiable* newReferentiable();
        Referentiable* newReferentiable(const std::string & nameHint, const std::vector<std::string> & guids);

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

        virtual Referentiable* newReferentiableInternal() = 0;
        virtual Referentiable* newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids) = 0;
        virtual ReferentiableNewCmdBase * CmdNew() = 0;
        virtual ReferentiableNewCmdBase * CmdNew(const std::string & nameHint, const std::vector<std::string> & guids) = 0;
    };

    template <class T>
    class ReferentiableManager : public ReferentiableManagerBase
    {
    public:
        static ReferentiableManager * getInstance();

        std::string defaultNameHint();

    private:
        static ReferentiableManager * g_pRefManager;

        ReferentiableManager();
        virtual ~ReferentiableManager();

        Referentiable* newReferentiableInternal() override;
        Referentiable* newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids) override;
        ReferentiableNewCmdBase * CmdNew() override;
        ReferentiableNewCmdBase * CmdNew(const std::string & nameHint, const std::vector<std::string> & guids) override;
    };

    class ReferentiableNewCmdBase : public Command
    {
    public:
        void getDescription(std::string & desc) override;

        Referentiable * refAddr() const;

    protected:
        ReferentiableNewCmdBase(const std::string & nameHint, const std::vector<std::string> guids);
        ReferentiableNewCmdBase();
        ~ReferentiableNewCmdBase();

        virtual ReferentiableManagerBase * manager() = 0;
    private:
        bool doExecute() override;
        void doUndo() override;
        void doRedo() override;

        struct parameters
        {
            std::string m_nameHint;
            std::vector<std::string> m_guids;
        };
        bool m_bHasParameters;
        parameters m_params;

        struct data
        {
            std::string m_GUID; // when undone and redone, the GUIDS must match
            std::string m_sessionName;
            Referentiable * m_addr;
        };
        data m_after;
    };

    template <class T>
    class ReferentiableNewCmd : public ReferentiableNewCmdBase
    {
        friend class ReferentiableManager < T > ;
    protected:
        ReferentiableNewCmd(const std::string & nameHint, const std::vector<std::string> guids);
        ReferentiableNewCmd();
        ~ReferentiableNewCmd();

        ReferentiableManagerBase * manager() override;
    private :
        ReferentiableManagerBase * m_manager;
    };
}