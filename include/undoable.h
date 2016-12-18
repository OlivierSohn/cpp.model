
namespace imajuscule
{
    class ReferentiableManagerBase;
    class Referentiable;
    class Command;
    class UndoGroup;
    class Undoable;
    typedef std::vector<std::unique_ptr<Undoable>> Undoables;
    class Undoable
    {
    public:
        
        enum State: unsigned char
        {
            NOT_EXECUTED,
            EXECUTED,
            UNDONE,
            REDONE
        };
        static const char * StateToString(State);

        virtual ~Undoable();
        // return false if Command has no effect
        virtual bool Execute() = 0;
        virtual bool Undo() = 0;
        virtual bool Redo() = 0;
        virtual bool Undo(Undoable * limit, bool bStrict, bool & bFoundLimit) = 0;
        virtual bool Redo(Undoable * limit, bool bStrict, bool & bFoundLimit) = 0;
        
        
        void Add(Undoable*); // adds an undoable in the list of sub element or to the current subelement if it exists (see StartSubElement)
        
        void StartSubElement();
        bool EndSubElement();
        
        void traverseForward(Undoables::iterator & begin, Undoables::iterator& end) const;

        void traverseForwardRecurse(std::vector<Undoable*> &) const;
        
//        bool contains(Undoable * u);
        
            
        bool isObsolete() const;

        void getNRExtendedDescription(std::string & desc) const;
        void getExtendedDescription(std::string & desc, size_t offset) const;
        virtual void getDescription(std::string & desc) const = 0;

        class CommandResult
        {
        public:
            CommandResult();
            CommandResult(bool bSuccess);
            virtual ~CommandResult() {}
            
            bool initialized() const;
            
            bool Success() const;
        private:
            bool m_bInitialized : 1;
            bool m_success : 1;
        };
        typedef std::function<void(const CommandResult *)> resFunc;

        enum class Event: unsigned char
        {
            RESULT,
            
            SIZE_ENUM
        };
        Observable<Event, const CommandResult *> & observable();

    protected:
        Undoable();

        void setIsObsolete();

        mutable Undoables m_undoables; // mutable because isObsolete() can delete commands

        struct data
        {
            bool operator==(const data&) const;
            virtual bool operator!=(const data&) const = 0;
            virtual std::string getDesc() const = 0;
            
            virtual ~data() {}
        };

        class CommandExec final
        {
        public:
            CommandExec(UndoGroup *, Command*, const resFunc *);
            
            bool Run();
        private:
            UndoGroup * m_group; // the group to which the command belongs
            Command* m_command; // the actual command
            const resFunc * m_pResFunc; // the result function for the actual command
        };

        template <class InnerCmdType>
        static bool ExecuteFromInnerCommand(const data & dataBefore, const data & dataAfter, Referentiable* ref =
                                            nullptr, const resFunc * pResFunc = nullptr);

        template <class InnerCmdType>
        bool ExecFromInnerCommand(const data & dataBefore, const data & dataAfter, Referentiable* ref = nullptr, const resFunc * pResFunc = nullptr);

        template <class InnerCmdType>
        std::vector<CommandExec> ListInnerCommandsReadyFor(const data & dataBefore, const data & dataAfter, Referentiable * ref = nullptr, const resFunc * pResFunc = nullptr);
        

    private:
        bool m_obsolete : 1;
        std::stack<Undoable*> m_curSubElts;
        Observable<Event, const CommandResult *> * m_observable;
    };
    
    // an UndoGroup is a way to sequence undoables for Undo/Redo
    class UndoGroup : public Undoable
    {
    public:        
        bool Execute() override;
        bool Undo() override;
        bool Redo() override;
        
        bool UndoUntil(Undoable*u /*including u*/);
        bool RedoUntil(Undoable*u /*including u*/);
        
        virtual bool isObsolete() const;
        
        void getDescription(std::string & desc) const override;
    private:
        
        bool Undo(Undoable * limit, bool bStrict, bool & bFoundLimit) override;
        bool Redo(Undoable * limit, bool bStrict, bool & bFoundLimit) override;
    };
    
    // A command has "inner" commands organized by groups (groups define a precedence constraints between their elements)
    // The initial intent was to have a single group containing all inner commands. But when redone / undone, the command wil need to call its sub commands in a non-linear way,
    // hence the need to have multiple groups.
    class Command : public Undoable
    {
        friend class Undoable;
        ////////////////////////////////
        /// original Command definition
        ////////////////////////////////
        
    public:
        enum ObsolescenceEvent: unsigned char
        {
            IS_OBSOLETE, // means that the command should not be considered by HistoryManager anymore
            
            SIZE_ENUM
        };
        
        virtual ~Command();
        
        State getState() const;
        bool validStateToExecute() const;
        bool validStateToUndo() const;
        bool validStateToRedo() const;
        
        bool Execute() override; // return false if Command has no effect
        bool Undo() override;
        bool Redo() override;
        
        void getDescription(std::string & desc) const override;
        virtual void getSentenceDescription(std::string & desc) const = 0;
        
    protected:
        virtual bool doExecute();
        virtual bool doUndo();
        virtual bool doRedo();
        
        void setState(State state);
        
    private:
        void onObsolete();
        
        State m_state;
        Observable<ObsolescenceEvent> * m_obsolescenceObservable;
        std::vector<FunctionInfo<ObsolescenceEvent>> m_reg;
        
        bool Undo(Undoable * limit, bool bStrict, bool & bFoundLimit) override;
        bool Redo(Undoable * limit, bool bStrict, bool & bFoundLimit) override;
        
    protected:
        Command(data * pDataBefore, data * pDataAfter, Referentiable * r = nullptr, Observable<ObsolescenceEvent> * o = nullptr);
        
        std::unique_ptr<data> m_pBefore, m_pAfter;
        
        data * Before() const;
        data * After() const;
        Referentiable * getObject() const;
        
    private:
        std::string m_guid;
        ReferentiableManagerBase * m_manager;
        
    protected:
        template <class InnerCmdType>
        bool ReadyFor(const data & dataBefore, const data & dataAfter, Referentiable * ref /*optional*/);
        
        virtual bool doExecute(const data & Data) = 0;
    };
}

// depending on where this MACRO is used, CommandResult will be XXX::CommandResult or YYY::CommandResult

#define RESULT_BY_REF(r) \
[&](const Command::CommandResult * res){                                \
A(res);                                                                 \
A(res->initialized());                                                 \
const CommandResult* myRes = dynamic_cast<const CommandResult*>(res);   \
A(myRes);                                                             \
r = *myRes;                                                        \
A(r.initialized()); \
}

#define SUBCR_LISTEN_TO_RESULT \
static FunctionInfo<Event> ListenToResult(Command & c, CommandResult & r) {         \
return c.observable().Register(Event::RESULT, RESULT_BY_REF(r));                    \
}

#define SUBCR \
public:                           \
CommandResult() = default; \
SUBCR_LISTEN_TO_RESULT;       \
private:



