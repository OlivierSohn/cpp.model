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

        virtual const char * defaultNameHint();
        Referentiable* newReferentiable();
        Referentiable* newReferentiable(const std::string & nameHint, const std::vector<std::string> & guids);

        // guid is unique
        Referentiable * findByGuid(const std::string & guid);
        // session name is unique per-session
        Referentiable * findBySessionName(const std::string & sessionName);

        void ListReferentiablesByCreationDate(referentiables& vItems);

        Observable<Event, Referentiable*> & observable();

        void RemoveRef(Referentiable*);

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
        void RemoveRefInternal(Referentiable*);
        virtual ReferentiableNewCmdBase * CmdNew() = 0;
        virtual ReferentiableNewCmdBase * CmdNew(const std::string & nameHint, const std::vector<std::string> & guids) = 0;
        virtual ReferentiableDeleteCmdBase * CmdDelete(const std::string & guid) = 0;
    };

    template <class T>
    class ReferentiableManager : public ReferentiableManagerBase
    {
        friend T;
    public:
        static ReferentiableManager * getInstance();

        const char * defaultNameHint();

    private:
        static ReferentiableManager * g_pRefManager;

        ReferentiableManager();
        virtual ~ReferentiableManager();

        Referentiable* newReferentiableInternal() override;
        Referentiable* newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids) override;
        virtual ReferentiableNewCmdBase * CmdNew() override;
        ReferentiableNewCmdBase * CmdNew(const std::string & nameHint, const std::vector<std::string> & guids) override;
        ReferentiableDeleteCmdBase * CmdDelete(const std::string & guid) override;
    };

    class ReferentiableCmdBase : public Command
    {
    public:
        Referentiable * refAddr() const;
        virtual ReferentiableManagerBase * manager() = 0;
        std::string guid() const;
        std::string hintName() const;

        virtual bool IsReadyToInstantiate() const = 0;
        virtual bool IsReadyToDeinstantiate() const = 0;

        virtual Referentiable * Instantiate() = 0;
        virtual void Deinstantiate() = 0;
    protected:
        struct data
        {
            std::string m_GUID; // when undone and redone, the GUIDS must match
            std::string m_hintName;
        };
        Referentiable * m_addr;
        data m_after;
    
        ReferentiableCmdBase();
        ~ReferentiableCmdBase();

        void doInstantiate();
        void doDeinstantiate();
    };
    class ReferentiableNewCmdBase : public ReferentiableCmdBase
    {
    public:
        void getDescription(std::string & desc) override;

        bool IsReadyToInstantiate() const;
        bool IsReadyToDeinstantiate() const;

        Referentiable * Instantiate() override;
        void Deinstantiate() override;
    
    protected:
        ReferentiableNewCmdBase(const std::string & nameHint, const std::vector<std::string> guids);
        ReferentiableNewCmdBase();
        ~ReferentiableNewCmdBase();

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
    };

    template <class T>
    class ReferentiableNewCmd : public ReferentiableNewCmdBase
    {
        friend class ReferentiableManager < T >;
    protected:
        ReferentiableNewCmd(const std::string & nameHint, const std::vector<std::string> guids);
        ReferentiableNewCmd();
        ~ReferentiableNewCmd();

        ReferentiableManagerBase * manager() override;
    private:
        ReferentiableManagerBase * m_manager;
    };

    class ReferentiableDeleteCmdBase : public ReferentiableCmdBase
    {
    public:
        void getDescription(std::string & desc) override;

        bool IsReadyToInstantiate() const;
        bool IsReadyToDeinstantiate() const;

        Referentiable * Instantiate() override;
        void Deinstantiate() override;
    protected:
        ReferentiableDeleteCmdBase(const std::string & guid);
        ~ReferentiableDeleteCmdBase();

    private:
        bool doExecute() override;
        void doUndo() override;
        void doRedo() override;

        struct parameters
        {
            std::string m_guid;
        };
        bool m_bHasParameters;
        parameters m_params;
    };

    template <class T>
    class ReferentiableDeleteCmd : public ReferentiableDeleteCmdBase
    {
        friend class ReferentiableManager < T >;
    protected:
        ReferentiableDeleteCmd(const std::string & guid);
        ~ReferentiableDeleteCmd();

        ReferentiableManagerBase * manager() override;
    private:
        ReferentiableManagerBase * m_manager;
    };
}

#include "referentiable.h"