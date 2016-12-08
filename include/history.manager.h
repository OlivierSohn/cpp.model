#pragma once

#include <list>
#include <stack>

#include "observable.h"
#include "execution.h"

namespace imajuscule
{
    class Command;
    class UndoGroup;
    class Undoable;
    class HistoryManager
    {
        friend class Globals;
    public:
        enum Event
        {
            UNDOS_CHANGED,
            REDOS_CHANGED
        };
        Observable<Event> & observable();
        void logCommand(Command*c, const char * pre = nullptr);
        void logObsoleteCommand(Command*c);

        typedef std::list<UndoGroup> UndoGroups;

        HistoryManager();
        virtual ~HistoryManager();
        void reset();
        
        static HistoryManager * getInstance();
        
        void PushPause();
        void PopPause();
        bool isActive() const;

        void Add(Undoable*);
        void StartTransaction();
        void EndTransaction();
        void MakeGroup();

        void Undo();
        void Redo();

        void PushCurrentCommand(Undoable*);
        void PopCurrentCommand(Undoable*);
        Undoable * CurrentCommand();
        bool IsUndoingOrRedoing(ExecutionType & t);

        // traverse in chronological order
        void traverseUndos(UndoGroups::const_iterator& begin, UndoGroups::const_iterator& end) const;
        void traverseRedos(UndoGroups::const_iterator& begin, UndoGroups::const_iterator& end) const;

    private:
        static HistoryManager * g_instance;

        bool m_bAppStateHasNewContent : 1;
        ExecutionType m_curExecType : 2;
        
        UndoGroups m_groups;
        Observable<Event> * m_observable;
        UndoGroups::reverse_iterator m_appState; //everything from rend to m_appState is "Done"
        unsigned int m_stacksCapacity;

        std::stack<Undoable*> m_curCommandStack;
        int m_iActivated;
        int transactionCount_ = 0;

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
    
    template <bool cond, class CLASS >
    class if_
    {
    public:
        if_() { if(cond) { class_ = new CLASS();} }
        ~if_() { if(cond) delete class_; }
    private:
        CLASS * class_;
    };
}

#include "undoable.h"
