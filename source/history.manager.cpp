#include "command.h"
#include "history.manager.h"
#include <iostream>
#include "os.log.h"
#include <algorithm>

using namespace imajuscule;

void HistoryManager::logObsoleteCommand(Command*c)
{
    logCommand(c, "[obsolete] ");
}

void HistoryManager::logCommand(Command*c, const char * pre)
{
    A(c);
    std::string pad;
    pad.insert(pad.end(), m_curCommandStack.size(), ' ');
    std::string desc;
    c->getExtendedDescription(desc);
    LG(INFO, "%s%s%s", pad.c_str(), pre ? pre : "", desc.c_str());
}

UndoGroup::UndoGroup():
Undoable()
{}

UndoGroup::~UndoGroup()
{
}

bool UndoGroup::isObsolete() const
{
    bool bEmpty = true;

    auto it = m_undoables.begin();
    auto end = m_undoables.end();
    while (it != end)
    {
        Undoable * u = *it;
        if (u->isObsolete())
        {
            //HistoryManager::getInstance()->logObsoleteCommand(c);
            delete u;
            it = m_undoables.erase(it);
        }
        else
        {
            bEmpty = false;
            ++it;
        }
    }

    return bEmpty;
}

bool UndoGroup::Execute()
{
    A(0);
    return false;
}

bool UndoGroup::Undo(){ bool bFoundLimit; return Undo(NULL, false, bFoundLimit); }
bool UndoGroup::UndoUntil(Undoable *limit){ bool bFoundLimit; return Undo(limit, true, bFoundLimit); }
bool UndoGroup::Undo(Undoable *limit, bool bStrict, bool & bFoundLimit)
{
    bool bNotEmpty = false;

    auto it = m_undoables.rbegin();
    auto end = m_undoables.rend();

    bFoundLimit = false;

    while(it != end)
    {
        Undoable * u = *it;
        if_A (u)
        {
            if (u->isObsolete())
            {
                //HistoryManager::getInstance()->logObsoleteCommand(c);
                delete u;
                it = std::reverse_iterator<Undoables::iterator>(m_undoables.erase((std::next(it)).base()));
                end = m_undoables.rend();
                continue;
            }

            bNotEmpty = true;

            //if (u->validStateToUndo()) // command can have been undone by a call to UndoUntil
            {
                /*bool bRelevant = */
                u->Undo(limit, false, bFoundLimit);
                // Assert commented out : a relevant (executed) command can become irrelevant for undo/redo e.g. SetFormula("", "0.")
                // -> should I introduce the notion of undoability?
                //    and have 2 different commands: ParamInitializeFormula(not undoable) and ParamChangeForula(undoable) ?
                //A(bRelevant);
            }
            
            if(bFoundLimit)
                break;
        }

        ++it;
    }

    if (bStrict)
        A(bFoundLimit);
    
    return bNotEmpty;
}
bool UndoGroup::Redo(){ bool bFoundLimit; return Redo(NULL, false, bFoundLimit); }
bool UndoGroup::RedoUntil(Undoable * limit){ bool bFoundLimit; return Redo(limit, true, bFoundLimit); }
bool UndoGroup::Redo(Undoable * limit, bool bStrict, bool & bFoundLimit)
{
    bool bNotEmpty = false;

    auto it = m_undoables.begin();
    auto end = m_undoables.end();
    bFoundLimit = false;

    while (it != end)
    {
        Undoable * u = *it;
        if_A(u)
        {
            if (u->isObsolete())
            {
                //HistoryManager::getInstance()->logObsoleteCommand(c);
                delete u;
                it = m_undoables.erase(it);
                continue;
            }

            bNotEmpty = true;

//            if(u->validStateToRedo())// command can have been undone by a call to RedoUntil
            {
                /*bool bRelevant =*/
                u->Redo(limit, false, bFoundLimit);
                // Assert commented out : a relevant (executed) command can become irrelevant for undo/redo e.g. SetFormula("", "0.")
                // -> should I introduce the notion of undoability?
                //    and have 2 different commands: ParamInitializeFormula(not undoable) and ParamChangeForula(undoable) ?
                //A(bRelevant);
            }
            
            if(bFoundLimit)
                break;
        }
    
        ++it;
    }

    if (bStrict)
        A(bFoundLimit);

    return bNotEmpty;
}


HistoryManager * HistoryManager::g_instance = NULL;

HistoryManager::HistoryManager() :
m_stacksCapacity(-1)// unsigned -> maximum capacity
, m_observable(Observable<Event>::instantiate())
, m_bAppStateHasNewContent(false)
, m_curExecType(Command::ExecType::NONE)
, m_bActivated(true)
{
    m_appState = m_groups.rbegin();
}

HistoryManager::~HistoryManager()
{
    EmptyStacks();

    m_observable->deinstantiate();
}

auto HistoryManager::observable()->Observable<Event> &
{
    return *m_observable;
}

void HistoryManager::Activate(bool bVal)
{
    m_bActivated = bVal;
}
bool HistoryManager::isActive() const
{
    return m_bActivated;
}
void HistoryManager::MakeGroup()
{
    A(NULL == CurrentCommand());

    if (m_bAppStateHasNewContent)
    {
        m_bAppStateHasNewContent = false;
    }
}
void HistoryManager::StartTransaction(){
    if (Command * cc = CurrentCommand())
        cc->StartSubElement();
}
void HistoryManager::EndTransaction(){
    if (Command * cc = CurrentCommand())
        cc->EndSubElement();
}


void HistoryManager::EmptyStacks()
{
    m_groups.clear();
    m_appState = m_groups.rbegin();
    m_bAppStateHasNewContent = false;
}

HistoryManager * HistoryManager::getInstance()
{
    if (!g_instance)
        g_instance = new HistoryManager();

    return g_instance;
}

Command * HistoryManager::CurrentCommand()
{
    if (m_curCommandStack.empty())
        return NULL;
    else
        return m_curCommandStack.top();
}

void HistoryManager::PushCurrentCommand(Command*c)
{
    //logCommand(c);
    m_curCommandStack.push(c);
}
void HistoryManager::PopCurrentCommand(Command*c)
{
    A(!m_curCommandStack.empty());
    A(m_curCommandStack.top() == c);
    m_curCommandStack.pop();
}

bool HistoryManager::IsUndoingOrRedoing(Command::ExecType & t)
{
    t = m_curExecType;
    return (t != Command::ExecType::NONE);
}

void HistoryManager::Add(Command* c)
{
    bool bRedosChanged = false;

    if (m_curExecType != Command::ExecType::NONE)
    {
        LG(ERR, "HistoryManager::Add : Memory leak : a command was added to history while %s", (m_curExecType==Command::ExecType::UNDO)?"undoing":"redoing");
        goto end;
    }

    if (!isActive())
    {
        // memory leak
        LG(ERR, "HistoryManager::Add : Memory leak : a command was added to history while inactive");
        goto end;
    }

    if (Command * cmd = CurrentCommand())
    {
        cmd->Add(c);
        goto end;
    }

    if (!m_bAppStateHasNewContent)
    {
        m_bAppStateHasNewContent = true;
        // erase "redos"
        auto firstRedoIt = m_appState.base();
        auto end = m_groups.end();
        if (firstRedoIt != end)
        {
            m_groups.erase(firstRedoIt, end);
            bRedosChanged = true;
        }

        // new group
        m_groups.emplace_back();
        m_appState = m_groups.rbegin();
    }

    m_appState->Add(c);
    A(!m_appState->isObsolete());
    SizeUndos();
    // TODO HistoryManager::Add : after SizeUndos, find an algo to recompute m_appState that could have been invalidated

    observable().Notify(Event::UNDOS_CHANGED);

    if (bRedosChanged)
    {
        observable().Notify(Event::REDOS_CHANGED);
    }

end:
    return;
}

void HistoryManager::SizeUndos()
{
    unsigned int size = m_groups.size();
    if (size > m_stacksCapacity)
    {
        unsigned int count = 0;
        auto end = m_groups.end();
        for (auto it = m_groups.begin(); it != end ;)
        {
            if (it->isObsolete())
            {
                count++;
                it = m_groups.erase(it);
            }
            else
                ++it;
        }

        LG(INFO, "HistoryManager::SizeUndos %u out of %u groups removed because empty", count, size);

        size = m_groups.size();
        if (size > m_stacksCapacity)
        {
            unsigned int nElementsRemoved = (size - m_stacksCapacity);
            UndoGroups::iterator it = m_groups.begin();
            m_groups.erase(it, std::next(it, nElementsRemoved));

            LG(INFO, "HistoryManager::SizeUndos %u first groups removed", nElementsRemoved);
        }
    }
}

void HistoryManager::Undo()
{
    m_curExecType = Command::ExecType::UNDO;

    bool bDone = false;
    bool bUndosChanged = false;
    bool bRedosChanged = false;

    while (m_appState != m_groups.rend() /*rend must be recomputed at each loop*/)
    {
        if (!m_appState->Undo())
        {
            // remove this empty state and increment
            bUndosChanged = true;

            m_appState = std::reverse_iterator<UndoGroups::iterator>(m_groups.erase(std::next(m_appState).base()));
        }
        else
        {
            ++m_appState;
            bDone = true;
            bRedosChanged = true;
            bUndosChanged = true;
            break;
        }
    }
    
    if (!bDone)
    {
        std::cout << "\a";
    }

    if (bUndosChanged)
    {
        observable().Notify(Event::UNDOS_CHANGED);
    }

    if (bRedosChanged)
    {
        observable().Notify(Event::REDOS_CHANGED);
    }

    m_curExecType = Command::ExecType::NONE;
}

void HistoryManager::Redo()
{
    m_curExecType = Command::ExecType::REDO;

    bool bDone = false;
    bool bUndosChanged = false;
    bool bRedosChanged = false;

    while (m_appState != m_groups.rbegin() /*rbegin must be recomputed at each loop*/ )
    {
        --m_appState;

        if (!m_appState->Redo())
        {
            // remove this empty state
            bRedosChanged = true;

            m_appState = std::reverse_iterator<UndoGroups::iterator>(m_groups.erase(std::next(m_appState).base()));
        }
        else
        {
            bDone = true;
            bRedosChanged = true;
            bUndosChanged = true;
            break;
        }
    }

    if (!bDone)
    {
        std::cout << "\a";
    }

    if (bUndosChanged)
    {
        observable().Notify(Event::UNDOS_CHANGED);
    }

    if (bRedosChanged)
    {
        observable().Notify(Event::REDOS_CHANGED);
    }

    m_curExecType = Command::ExecType::NONE;
}

void HistoryManager::traverseUndos(UndoGroups::const_iterator& begin, UndoGroups::const_iterator& end) const
{
    begin = m_groups.begin();
    end = m_appState.base();
}
void HistoryManager::traverseRedos(UndoGroups::const_iterator& begin, UndoGroups::const_iterator& end) const
{
    begin = m_appState.base();
    end = m_groups.end();
}
