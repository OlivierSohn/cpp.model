#pragma once

#include <deque>
#include "observable.h"

namespace imajuscule
{
    class Command;
    class HistoryManager
    {
    public:
        enum Event
        {
            UNDOS_CHANGED,
            REDOS_CHANGED
        };
        Observable<Event> & observable();
        
        typedef std::deque<Command*> RedoStack;
        typedef std::deque<Command*> UndoStack;

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

        // traverse in chronological order (hence reverse iterator for Redos)
        unsigned int traverseUndos(UndoStack::const_iterator& begin, UndoStack::const_iterator& end);
        unsigned int traverseRedos(RedoStack::const_reverse_iterator& begin, RedoStack::const_reverse_iterator& end);

    private:
        UndoStack m_undos;
        RedoStack m_redos;

        Observable<Event> * m_observable;

        void EmptyRedos();
        void EmptyUndos();
        void SizeUndos();

        static HistoryManager * g_instance;
        unsigned int m_stacksCapacity;
    };
}