#pragma once

#include <list>
#include <stack>
#include "observable.h"

namespace imajuscule
{
    class Command;

    class UndoGroup
    {
    public:
        UndoGroup();
        ~UndoGroup();

        void Add(Command*);

        bool isObsolete();

        // Undo and Redo return false if nothing changed (the group was composed of obsolete commands)
        bool Undo();
        bool Redo();

        typedef std::list<Command*> Commands;
        void traverseForward(Commands::const_iterator & it, Commands::const_iterator & end) const;
    private:
        Commands m_commands;
    };

    class HistoryManager
    {
    public:
        enum Event
        {
            UNDOS_CHANGED,
            REDOS_CHANGED
        };
        Observable<Event> & observable();
        void logCommand(Command*c, const char * pre = NULL);
        void logObsoleteCommand(Command*c);

        typedef std::list<UndoGroup> UndoGroups;

        HistoryManager();
        virtual ~HistoryManager();
        static HistoryManager * getInstance();

        void Activate(bool);
        bool isActive() const;
        void EmptyStacks();

        void Add(Command*);
        void MakeGroup();

        void Undo();
        void Redo();

        void PushCurrentCommand(Command*);
        void PopCurrentCommand(Command*);
        Command * CurrentCommand();
        bool IsUndoingOrRedoing();

        // traverse in chronological order
        void traverseUndos(UndoGroups::const_iterator& begin, UndoGroups::const_iterator& end) const;
        void traverseRedos(UndoGroups::const_iterator& begin, UndoGroups::const_iterator& end) const;

    private:
        static HistoryManager * g_instance;

        UndoGroups m_groups;
        Observable<Event> * m_observable;
        UndoGroups::reverse_iterator m_appState; //everything from rend to m_appState is "Done"
        unsigned int m_stacksCapacity;
        bool m_bAppStateHasNewContent;

        bool m_bIsUndoingOrRedoing;
        std::stack<Command*> m_curCommandStack;
        bool m_bActivated;

        void NewGroup();
        void SizeUndos();
    };
}