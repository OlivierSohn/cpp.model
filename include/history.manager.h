#pragma once

#include <stack>

namespace imajuscule
{
    class Command;
    class HistoryManager
    {
    public:
        HistoryManager();
        virtual ~HistoryManager();
        static HistoryManager * getInstance();

        void setStackCapacity(unsigned int);
        void EmptyStacks();

        void Add(Command*);

        unsigned int CountUndos();
        unsigned int CountRedos();
        void Undo();
        void Redo();

    private:
        typedef std::deque<Command*> RedoStack;
        typedef std::deque<Command*> UndoStack;
        UndoStack m_undos;
        RedoStack m_redos;

        void EmptyRedos();
        void EmptyUndos();
        void SizeUndos();

        static HistoryManager * g_instance;
        unsigned int m_stacksCapacity;
    };
}