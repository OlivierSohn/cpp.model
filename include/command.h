#pragma once

#include <string>
#include "observable.h"
#include "os.log.h"

namespace imajuscule
{
    class HistoryManager;
    class Command;
    typedef std::vector<Command*> Commands;

    class Command
    {
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
        virtual void getDescription(std::string & desc) = 0;
    protected:
        Command(Observable<ObsolescenceEvent> * o = NULL);
        bool isInnerCommand(Command * c);

    private:
        virtual bool doExecute() = 0;
        virtual void doUndo() = 0;
        virtual void doRedo() = 0;

        void onObsolete();

        State m_state;
        bool m_obsolete;
        Observable<ObsolescenceEvent> * m_obsolescenceObservable;
        std::vector<FunctionInfo<ObsolescenceEvent>> m_reg;

        Commands m_innerCommands;

        HistoryManager * getHistoryManager();
    };
}

#include "history.manager.h"

#define inCmd (HistoryManager::getInstance()->CurrentCommand())
