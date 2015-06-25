#pragma once

#include <list>
#include <stack>
#include "observable.h"
#include "undoable.h"
#include "command.h"

namespace imajuscule
{
    class Command;

    // an UndoGroup is a way to sequence undoables for Undo/Redo
    class UndoGroup : public Undoable
    {
    public:
        UndoGroup();
        ~UndoGroup();

        bool Execute() override;
        bool Undo() override;
        bool Redo() override;

        bool UndoUntil(Undoable*u /*including u*/);
        bool RedoUntil(Undoable*u /*including u*/);
    
        virtual bool isObsolete() const;

    private:

        bool Undo(Undoable * limit, bool bStrict, bool & bFoundLimit) override;
        bool Redo(Undoable * limit, bool bStrict, bool & bFoundLimit) override;
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
        
        void PushPause();
        void PopPause();
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
        int m_iActivated;

        void NewGroup();
        void SizeUndos();
    };
    
    class HistoryManagerTransaction
    {
    public:
        HistoryManagerTransaction();
        ~HistoryManagerTransaction();
    };
    class HistoryManagerPause
    {
    public:
        HistoryManagerPause();
        ~HistoryManagerPause();
    };
}

#define inCmd (HistoryManager::getInstance()->CurrentCommand())
