#pragma once

#include <string>
#include "observable.h"

namespace imajuscule
{
    class HistoryManager;
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

        void Execute();
        void Undo();
        void Redo();

        State getState();
        bool isObsolete();

        virtual void getDescription(std::string & desc) = 0;
    protected:
        Command(Observable<ObsolescenceEvent> * o = NULL);
    private:
        virtual bool doExecute() = 0;
        virtual void doUndo() = 0;
        virtual void doRedo() = 0;

        void onObsolete();

        State m_state;
        bool m_obsolete;
        HistoryManager * m_history;
        Observable<ObsolescenceEvent> * m_obsolescenceObservable;
        std::vector<FunctionInfo<ObsolescenceEvent>> m_reg;

        HistoryManager * getHistoryManager();
    };
}