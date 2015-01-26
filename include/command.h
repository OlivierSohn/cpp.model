#pragma once

#include <string>

namespace imajuscule
{
    class HistoryManager;
    class Command
    {
        friend class HistoryManager;
    public:
        enum State
        {
            NOT_EXECUTED,
            EXECUTED,
            UNDO,
            REDO
        };
        static const char * StateToString(State);

        void Execute();
        void Undo();
        void Redo();

        State getState();

        virtual void getDescription(std::string & desc) = 0;
    protected:
        Command(HistoryManager * hm = NULL);
        virtual ~Command();
    private:
        virtual bool doExecute() = 0;
        virtual void doUndo() = 0;
        virtual void doRedo() = 0;

        State m_state;
        HistoryManager * m_history;
        HistoryManager * getHistoryManager();
    };
}