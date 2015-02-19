#pragma once

#include <list>
#include <stack>
#include "observable.h"
#include "undoable.h"
#include "command.h"

namespace imajuscule
{
    class Command;

    // an UndoGroup is a way to order commands for Undo/Redo
    class UndoGroup : public Undoable
    {
    public:
        UndoGroup();
        ~UndoGroup();

        void Add(Command*) override;

        bool Execute() override;
        bool Undo() override;
        bool Redo() override;

        bool UndoUntil(Command*c /*including c*/);
        bool RedoUntil(Command*c /*including c*/);

        typedef std::list<Command*> Commands;
        void traverseForward(Commands::const_iterator & it, Commands::const_iterator & end) const;
    
        virtual bool isObsolete() const;

    private:
        mutable Commands m_commands; // mutable because isObsolete() can delete commands

        bool UndoInternal(Command * limit, bool bStrict = false);
        bool RedoInternal(Command * limit, bool bStrict = false);
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
        void StartTransaction();
        void EndTransaction();
        void MakeGroup();

        void Undo();
        void Redo();

        void PushCurrentCommand(Command*);
        void PopCurrentCommand(Command*);
        Command * CurrentCommand();
        bool IsUndoingOrRedoing(Command::ExecType & t);

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

        Command::ExecType m_curExecType;
        std::stack<Command*> m_curCommandStack;
        bool m_bActivated;

        void NewGroup();
        void SizeUndos();
    };
}

#define inCmd (HistoryManager::getInstance()->CurrentCommand())
