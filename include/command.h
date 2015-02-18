#pragma once

#include <string>
#include "observable.h"
#include "os.log.h"
#include "referentiable.h"

namespace imajuscule
{
    class HistoryManager;
    class Command;
    typedef std::vector<Command*> Commands;

    class Command
    {
        ////////////////////////////////
        /// original Command definition
        ////////////////////////////////

    public:
        enum ObsolescenceEvent
        {
            IS_OBSOLETE // means that the command should not be considered by HistoryManager anymore
        };
        enum State
        {
            NOT_EXECUTED,
            EXECUTED,
            UNDO,
            REDO
        };
        static const char * StateToString(State);

        virtual ~Command();

        bool Execute(); // return false if Command has no effect
        void Undo();
        void Redo();
        void addInnerCommand(Command*);

        void traverseInnerCommands(Commands::iterator & begin, Commands::iterator & end);

        State getState() const;
        bool isObsolete() const;

        bool validStateToUndo() const;
        bool validStateToRedo() const;

        void getExtendedDescription(std::string & desc);
        void getDescription(std::string & desc);
        virtual void getSentenceDescription(std::string & desc) = 0;

    protected:
        bool isInnerCommand(Command * c);

        virtual bool doExecute();
        virtual void doUndo();
        virtual void doRedo();

    private:
        void onObsolete();

        State m_state;
        bool m_obsolete;
        Observable<ObsolescenceEvent> * m_obsolescenceObservable;
        std::vector<FunctionInfo<ObsolescenceEvent>> m_reg;

        Commands m_innerCommands;

    public:
        ////////////////////////////////
        /// generic referentiable Command definition
        ////////////////////////////////

        class CommandResult
        {
        public:
            CommandResult();
            CommandResult(bool bSuccess);
            virtual ~CommandResult();

            bool initialized() const;

            bool Success() const;
        private:
            bool m_bInitialized;
            bool m_success;
        };

        enum Event
        {
            RESULT
        };
        typedef std::function<void(const CommandResult *)> resFunc;
        Observable<Event, const CommandResult *> & observable();
    protected:
        struct data
        {
            bool operator==(const data&) const;
            virtual bool operator!=(const data&) const = 0;
            virtual std::string getDesc() const = 0;

            ~data();
            data();
        };
    protected:
        Command(data * pDataBefore, data * pDataAfter, Referentiable * r = NULL, Observable<ObsolescenceEvent> * o = NULL);

        data *m_pBefore;
        data *m_pAfter;

        data * Before() const;
        data * After() const;
        Referentiable * getObject() const;

    private:
        std::string m_guid;
        ReferentiableManagerBase * m_manager;
        Observable<Event, const CommandResult *> * m_observable;

        struct CommandExec
        {
            Command* m_command;
            enum Type
            {
                UNDO,
                REDO
            };
            Type m_type;

            CommandExec(Command*, Type);
        };
    public:
        bool ExecFromInnerCommand(const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable* ref = NULL, const resFunc * pResFunc = NULL);
    protected:
        static bool ExecuteFromInnerCommand(const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable* ref = NULL, const resFunc * pResFunc = NULL);
        std::vector<CommandExec> ListInnerCommandsReadyFor(const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable * ref = NULL);
        virtual bool ReadyFor(const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable * ref /*optional*/, CommandExec::Type & t);

        virtual bool doExecute(const data & Data) = 0;
    };
}

#include "history.manager.h"

#define inCmd (HistoryManager::getInstance()->CurrentCommand())


// depending on where this MACRO is used, CommandResult will be XXX::CommandResult or YYY::CommandResult

#define RESULT_BY_REF(r) \
       [&](const Command::CommandResult * res){                                \
            if_A(res)                                                                 \
            {                                                                           \
                if_A(res->initialized());                                                 \
                {                                                                          \
                    const CommandResult* myRes = dynamic_cast<const CommandResult*>(res);   \
                    if_A(myRes)                                                             \
                    {                                                                       \
                        r = *myRes;                                                        \
                        A(r.initialized());                                                \
                    }                                                                     \
                }                                                                       \
            }                                                                          \
        }

#define SUBCR_LISTEN_TO_RESULT \
    static FunctionInfo<Event> ListenToResult(Command & c, CommandResult & r)       \
    {                                                                               \
        return c.observable().Register(Event::RESULT, RESULT_BY_REF(r));            \
    }

#define SUBCR_DFLT_CONTRUCTOR \
    CommandResult() : Command::CommandResult() {}

#define SUBCR_DESTRUCTOR  \
    ~CommandResult() {}

#define SUBCR \
    public:                           \
        SUBCR_DFLT_CONTRUCTOR;        \
        SUBCR_DESTRUCTOR;             \
        SUBCR_LISTEN_TO_RESULT;       \
    private:

