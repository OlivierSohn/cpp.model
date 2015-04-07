#pragma once

#include <string>
#include <list>
#include "observable.h"
#include "referentiable.h"
#include "undoable.h"

namespace imajuscule
{
    class HistoryManager;
    class UndoGroup;

    // A command has "inner" commands organized by groups (groups define a precedence constraints between their elements)
    // The initial intent was to have a single group containing all inner commands. But when redone / undone, the command wil need to call its sub commands in a non-linear way,
    // hence the need to have multiple groups.
    class Command : public Undoable
    {
        ////////////////////////////////
        /// original Command definition
        ////////////////////////////////

    public:
        enum ObsolescenceEvent
        {
            IS_OBSOLETE // means that the command should not be considered by HistoryManager anymore
        };

        virtual ~Command();


        bool Execute() override; // return false if Command has no effect
        bool Undo() override;
        bool Redo() override;

        void Add(Command*) override;
        void startTransaction();
        void endTransaction();

        void getExtendedDescription(std::string & desc);
        void getDescription(std::string & desc);
        virtual void getSentenceDescription(std::string & desc) = 0;

    protected:
        virtual bool doExecute();
        virtual bool doUndo();
        virtual bool doRedo();

    private:
        void onObsolete();

        Observable<ObsolescenceEvent> * m_obsolescenceObservable;
        std::vector<FunctionInfo<ObsolescenceEvent>> m_reg;

        typedef std::list<UndoGroup*> UndoGroups;
        UndoGroups m_innerGroups;
        bool m_bInTransaction;
        UndoGroup * m_curGroup;

        void traverseInnerGroups(UndoGroups::iterator&begin, UndoGroups::iterator&end);

    public:
        ////////////////////////////////
        /// generic referentiable Command definition
        ////////////////////////////////

        enum ExecType
        {
            UNDO,
            REDO,
            NONE
        };

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

            virtual ~data();
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

        class CommandExec
        {
        public:
            CommandExec(UndoGroup *, Command*, ExecType, const resFunc *);
            virtual ~CommandExec();

            bool Run();
        private:
            UndoGroup * m_group; // the group to which the command belongs
            Command* m_command; // the actual command
            const resFunc * m_pResFunc; // the result function for the actual command
            ExecType m_type; // the type of execution
        };
    public:
        bool ExecFromInnerCommand(ExecType t, const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable* ref = NULL, const resFunc * pResFunc = NULL);
    protected:
        static bool ExecuteFromInnerCommand(const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable* ref = NULL, const resFunc * pResFunc = NULL);
        std::vector<CommandExec> ListInnerCommandsReadyFor(ExecType t, const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable * ref = NULL, const resFunc * pResFunc = NULL);
        virtual bool ReadyFor(ExecType t, const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable * ref /*optional*/);

        virtual bool doExecute(const data & Data) = 0;
    };
}

// headers for macros

#include "os.log.h"
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

