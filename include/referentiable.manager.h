#pragma once

#include <map>
#include <string>
#include <vector>

#include "visitable.h"
#include "observable.h"
#include "command.h"

namespace imajuscule
{
    int InitializeRefManagers();

    class ReferentiableCmdBase;
    class ReferentiableNewCmdBase;
    class ReferentiableDeleteCmdBase;

    class Referentiable;
    typedef std::vector<Referentiable*> referentiables;
    class ReferentiableManagerBase : public Visitable
    {
        friend class Referentiable;
        friend class ReferentiableCmdBase;
        friend class ReferentiableNewCmdBase;
        friend class ReferentiableDeleteCmdBase;
    public:
        enum class Event
        {
            RFTBL_ADD, // a referentiable was added to the list of referentiables managed by the manager
            RFTBL_REMOVE, // a referentiable was removed from the list of referentiables managed by the manager
            MANAGER_DELETE// the manager is being deleted
        };
        ReferentiableManagerBase();
        virtual ~ReferentiableManagerBase();

        Referentiable* newReferentiable();
        Referentiable* newReferentiable(const std::string & nameHint);
        Referentiable* newReferentiable(const std::string & nameHint, const std::vector<std::string> & guids);

        // guid is unique
        Referentiable * findByGuid(const std::string & guid);
        // session name is unique per-session
        Referentiable * findBySessionName(const std::string & sessionName);

        void ListReferentiablesByCreationDate(referentiables& vItems);

        Observable<Event, Referentiable*> & observable();

        void RemoveRef(Referentiable*);
        
        virtual unsigned int index() = 0;

        PERSISTABLE_VISITOR_HEADER_IMPL

    protected:
        virtual const char * defaultNameHint();
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

        virtual Referentiable* newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids, bool bVisible = true) = 0;
        void RemoveRefInternal(Referentiable*);
    };
    
    class Referentiables
    {
    public:
        static Referentiable* fromGUID(const std::string &);
        static void registerManager(ReferentiableManagerBase &);
    private:
        Referentiables();
        virtual ~Referentiables();
        static Referentiables * getInstance();
        static Referentiables * m_instance;
        std::vector<ReferentiableManagerBase*> m_managers;

        Referentiable* findRefFromGUID(const std::string &);
        void regManager(ReferentiableManagerBase &);
    };

    template <class T>
    class ReferentiableManager : public ReferentiableManagerBase
    {
        friend T;
    public:
        static ReferentiableManager * getInstance();

        const char * defaultNameHint();
        
        unsigned int index() override;

    private:
        static ReferentiableManager * g_pRefManager;

        ReferentiableManager();
        virtual ~ReferentiableManager();

        Referentiable* newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids, bool bVisible) override;
    };

    class ReferentiableCmdBase : public Command
    {
    public:
        Referentiable * refAddr() const;
        ReferentiableManagerBase * manager() const;
        std::string guid() const;
        std::string hintName() const;

        virtual void Instantiate() = 0;
        virtual void Deinstantiate() = 0;
    protected:
        enum Action
        {
            ACTION_NEW,
            ACTION_DELETE,
            ACTION_UNKNOWN
        };
        static Action other(Action);
        struct data : public Command::data
        {
            Action m_action;
            std::string m_hintName;
            ReferentiableManagerBase * m_manager;

            bool operator!=(const Command::data& other) const override;
            std::string getDesc() const override;

            data(Action, std::string hintName, ReferentiableManagerBase * rm);
            static data * instantiate(Action, std::string hintName, ReferentiableManagerBase * rm);
        };

        std::string m_hintName;
        std::string m_GUID;
    
        ReferentiableCmdBase(ReferentiableManagerBase * manager, const std::string & nameHint, Action action);
        ~ReferentiableCmdBase();

        void doInstantiate();
        void doDeinstantiate();

        bool doExecute(const Command::data &) override;

        class CommandResult : public Command::CommandResult
        {
            SUBCR;
        public:
            CommandResult(bool bSuccess, Referentiable*);

            Referentiable * addr() const;
        private:
            Referentiable * m_addr;
        };

    private:
        ReferentiableManagerBase * m_manager;
    };
    class ReferentiableNewCmdBase : public ReferentiableCmdBase
    {
        friend class ReferentiableManagerBase;
    public:
        void getSentenceDescription(std::string & desc) override;

        void Instantiate() override;
        void Deinstantiate() override;
    
    protected:
        ReferentiableNewCmdBase(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids);
        ~ReferentiableNewCmdBase();

        std::vector<std::string> m_guids;
    private:
        static bool ExecuteFromInnerCommand(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids, Referentiable*& oRefAddr);
        static Referentiable* Execute(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids);
    };

    class ReferentiableDeleteCmdBase : public ReferentiableCmdBase
    {
        friend class ReferentiableManagerBase;
    public:
        void getSentenceDescription(std::string & desc) override;

        void Instantiate() override;
        void Deinstantiate() override;

    protected:
        ReferentiableDeleteCmdBase(Referentiable&);
        ~ReferentiableDeleteCmdBase();

    private:
        static bool ExecuteFromInnerCommand(Referentiable&);
        static void Execute(Referentiable &);
    };

}

#include "referentiable.h"