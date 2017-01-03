
#define MAKE_REF(x) intrusive_ptr<x>(ReferentiableManager<x>::New().release())
namespace imajuscule
{
    class Referentiables;
    int InitializeRefManagers(Referentiables &);

    class ReferentiableCmdBase;
    class ReferentiableNewCmdBase;
    class ReferentiableDeleteCmdBase;

    typedef std::vector<Referentiable*> referentiables;
    
    class ReferentiableManagerBase : public Visitable
    {
        friend class Referentiable;
        friend class ReferentiableCmdBase;
        friend class ReferentiableNewCmdBase;
        friend class ReferentiableDeleteCmdBase;
    public:
        enum class Event: unsigned char
        {
            RFTBL_ADD, // a referentiable was added to the list of referentiables managed by the manager
            RFTBL_REMOVE, // a referentiable was removed from the list of referentiables managed by the manager
            MANAGER_DELETE, // the manager is being deleted
            
            SIZE_ENUM
        };
        ReferentiableManagerBase();
        
        // at this point, all ReferentiableManagers are live
        void teardown() {doTearDown();}
        
        // at this point, some ReferentiableManagers have been deleted already
        virtual ~ReferentiableManagerBase();

        ref_unique_ptr<Referentiable> newReferentiable(bool bFinalize);
        ref_unique_ptr<Referentiable> newReferentiable(const std::string & nameHint, bool bFinalize);
        ref_unique_ptr<Referentiable> newReferentiable(const std::string & nameHint, const std::vector<std::string> & guids, bool bFinalize);

        auto const & traverse() const { return refs; }
        void forEach(std::function<void(Referentiable &)> && f) {
            for(auto r : traverse()) {
                if(r) {
                    f(*r);
                }
            }
        }

        // guid is unique
        Referentiable * findByGuid(const std::string & guid);
        // session name is unique per-session
        Referentiable * findBySessionName(const std::string & sessionName);

        referentiables ListReferentiablesByCreationDate() const;

        Observable<Event, Referentiable*> & observable() {
            return *m_observable;
        }

        void RemoveRef(Referentiable*);
        
        virtual unsigned int index() = 0;

        VISITOR_HEADER_IMPL

        virtual const char * UIName() = 0;

        static std::string generateGuid();

        virtual const char * defaultNameHint() = 0;
    protected:
        // pure virtual because the session names are unique "per object type"
        bool ComputeSessionName(Referentiable*, bool bFinalize);

        bool RegisterWithSessionName(Referentiable*, const std::string& sessionName);
        
        virtual void doTearDown() = 0;
    private:
        // guid - referentiable
        typedef std::map<std::string, Referentiable*> guidsToRftbls;
        // session name - referentiable
        typedef std::map<std::string, Referentiable*> snsToRftbls;

        snsToRftbls m_snsToRftbls;
        guidsToRftbls m_guidsToRftbls;
        std::vector<Referentiable*> refs;
        
        int32_t session_name_last_suffix;

        Observable<Event, Referentiable*> * m_observable;

        virtual ref_unique_ptr<Referentiable> newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids, bool bFinalize = true) = 0;
        void RemoveRefInternal(Referentiable*);
    };
    
    template <class T>
    class ReferentiableManager : public ReferentiableManagerBase
    {
        friend class Globals;
        friend T;
    public:
        static ReferentiableManager * getInstance();

        const char * defaultNameHint() override;
        const char * UIName() override;
        
        unsigned int index() override;
        
        static ref_unique_ptr<T> New();

        void forEachReferentiable(std::function<void(T&)> && f) {
            for(auto r : traverse()) {
                auto * cast = safe_cast<T*>(r);
                f(*cast);
            }
        }

    private:
        static ReferentiableManager * g_pRefManager;

        ref_unique_ptr<Referentiable> newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids, bool bFinalize) override;
        void doTearDown() override {}
    };
    
    template <class T>
    void forEach(std::function<void(T&)> && f) {
        auto rm = ReferentiableManager<T>::getInstance();
        if(!rm) {
            return;
        }
        rm->forEachReferentiable(std::move(f));
    }

    class ReferentiableCmdBase : public Command
    {
    public:
        Referentiable * refAddr() const;
        ReferentiableManagerBase * manager() const;
        std::string const & guid() const;
        std::string const & hintName() const;

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
        
        struct data : public Undoable::data {
            Action m_action;
            std::string m_hintName;
            ReferentiableManagerBase * m_manager;

            bool operator!=(const Undoable::data& other) const override;
            std::string getDesc() const override;

            data(Action, std::string hintName, ReferentiableManagerBase * rm);
            static data * instantiate(Action, std::string hintName, ReferentiableManagerBase * rm);
        };

        std::string m_hintName;
        std::string m_GUID;
    
        ReferentiableCmdBase(ReferentiableManagerBase * manager, const std::string & nameHint, Action action);

        void doInstantiate();
        void doDeinstantiate();

        bool doExecute(const Undoable::data &) override;

        class CommandResult : public Undoable::CommandResult
        {
            SUBCR
        public:
            CommandResult(bool bSuccess, Referentiable*);

            ref_unique_ptr<Referentiable> addr() const {
                return {m_addr};
            }
        private:
            Referentiable * m_addr; // doc F3F7C744-0B78-4750-A0A1-7A9BAD872188
        };

    private:
        ReferentiableManagerBase * m_manager;
    };
    
    class ReferentiableNewCmdBase : public ReferentiableCmdBase
    {
        friend class ReferentiableManagerBase;
    public:
        void getSentenceDescription(std::string & desc) const override;

        void Instantiate() override;
        void Deinstantiate() override;
    
    protected:
        ReferentiableNewCmdBase(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids);

        std::vector<std::string> m_guids;
    private:
        static bool ExecuteFromInnerCommand(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids, ref_unique_ptr<Referentiable>& oRefAddr);
        static ref_unique_ptr<Referentiable> Execute(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids);
    };

    class ReferentiableDeleteCmdBase : public ReferentiableCmdBase
    {
        friend class ReferentiableManagerBase;
    public:
        void getSentenceDescription(std::string & desc) const override;

        void Instantiate() override;
        void Deinstantiate() override;

    protected:
        ReferentiableDeleteCmdBase(Referentiable&);

    private:
        static bool ExecuteFromInnerCommand(Referentiable&);
        static void Execute(Referentiable &);
    };
    
    template <class T>
    T* REF_BY_SN( const std::string & sn ){
        auto rm = ReferentiableManager<T>::getInstance();
        if(!rm) {
            return {};
        }
        return static_cast<T *>(rm->findBySessionName(sn));
    }
    
    template <class T>
    T* REF_BY_GUID(const std::string & guid) {
        auto rm = ReferentiableManager<T>::getInstance();
        if(!rm) {
            return {};
        }
        return static_cast<T *>(rm->findByGuid(guid));
    }
    
    template <class T> referentiables const & TRAVERSE() {
        auto rm = ReferentiableManager<T>::getInstance();
        if(!rm) {
            static referentiables rs;
            return rs;
        }

        return rm->traverse();
    }
}

