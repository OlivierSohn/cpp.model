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

        enum State
        {
            NOT_EXECUTED,
            EXECUTED,
            UNDONE,
            REDONE
        };
        static const char * StateToString(State);
        
        State getState() const;
        bool validStateToExecute() const;
        bool validStateToUndo() const;
        bool validStateToRedo() const;

        bool Execute() override; // return false if Command has no effect
        bool Undo() override;
        bool Redo() override;

        virtual void getDescription(std::string & desc);
        virtual void getSentenceDescription(std::string & desc) = 0;

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

            virtual ~data();
            data();
        };

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
            CommandExec(UndoGroup *, Command*, const resFunc *);
            virtual ~CommandExec();

            bool Run();
        private:
            UndoGroup * m_group; // the group to which the command belongs
            Command* m_command; // the actual command
            const resFunc * m_pResFunc; // the result function for the actual command
        };
    public:
        template <class InnerCmdType>
        bool ExecFromInnerCommand(const data & dataBefore, const data & dataAfter, Referentiable* ref = NULL, const resFunc * pResFunc = NULL);

    protected:
        template <class InnerCmdType>
        static bool ExecuteFromInnerCommand(const data & dataBefore, const data & dataAfter, Referentiable* ref = NULL, const resFunc * pResFunc = NULL);
    
        template <class InnerCmdType>
        std::vector<CommandExec> ListInnerCommandsReadyFor(const data & dataBefore, const data & dataAfter, Referentiable * ref = NULL, const resFunc * pResFunc = NULL);
        
        template <class InnerCmdType>
        bool ReadyFor(const data & dataBefore, const data & dataAfter, Referentiable * ref /*optional*/);

        virtual bool doExecute(const data & Data) = 0;
    };
}

#include "command.hpp"
