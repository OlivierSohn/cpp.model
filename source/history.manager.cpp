#include "command.h"
#include "history.manager.h"
#include "os.log.h"
#include <iostream>
#include <cassert>
#include <algorithm>

using namespace imajuscule;

UndoGroup::UndoGroup()
{}

UndoGroup::~UndoGroup()
{
    auto it = m_commands.begin();
    auto end = m_commands.end();
    for (; it != end; ++it)
    {
        delete *it;
    }
}

void UndoGroup::traverseForward(Commands::const_iterator & it, Commands::const_iterator & end) const
{
    it = m_commands.begin();
    end = m_commands.end();
}

bool UndoGroup::isObsolete()
{
    bool bEmpty = true;

    auto it = m_commands.begin();
    auto end = m_commands.end();
    while (it != end)
    {
        Command * c = *it;
        if (c->isObsolete())
        {
            delete c;
            it = m_commands.erase(it);
        }
        else
        {
            bEmpty = false;
            ++it;
        }
    }

    return bEmpty;
}

void UndoGroup::Add(Command*c)
{
    m_commands.push_back(c);
}

bool UndoGroup::Undo()
{
    bool bNotEmpty = false;

    auto it = m_commands.rbegin();
    auto end = m_commands.rend();

    while(it != end)
    {
        Command * c = *it;
        if (c)
        {
            if (c->isObsolete())
            {
                delete c;
                it = std::reverse_iterator<Commands::iterator>(m_commands.erase((std::next(it)).base()));
                end = m_commands.rend();
                continue;
            }

            bNotEmpty = true;

            Command::State s = c->getState();
            if ((s == Command::EXECUTED) || (s == Command::REDO))
            {
                c->Undo();
            }
            else
            {
                LG(ERR, "UndoGroup::Undo : Command in state %s", Command::StateToString(s));
                assert(0);
            }
        }
        else
        {
            LG(ERR, "UndoGroup::Undo : NULL Command");
            assert(0);
        }

        ++it;
    }

    return bNotEmpty;
}
bool UndoGroup::Redo()
{
    bool bNotEmpty = false;

    auto it = m_commands.begin();
    auto end = m_commands.end();

    while (it != end)
    {
        Command * c = *it;
        if (c)
        {
            if (c->isObsolete())
            {
                delete c;
                it = m_commands.erase(it);
                continue;
            }

            bNotEmpty = true;

            Command::State s = c->getState();
            if (s == Command::UNDO)
            {
                c->Redo();
            }
            else
            {
                LG(ERR, "UndoGroup::Redo : Command in state %s", Command::StateToString(s));
                assert(0);
            }
        }
        else
        {
            LG(ERR, "UndoGroup::Redo : NULL Command");
            assert(0);
        }
    
        ++it;
    }

    return bNotEmpty;
}


HistoryManager * HistoryManager::g_instance = NULL;

HistoryManager::HistoryManager() :
m_stacksCapacity(-1)// unsigned -> maximum capacity
, m_observable(Observable<Event>::instantiate())
, m_bAppStateHasNewContent(false)
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

void HistoryManager::MakeGroup()
{
    if (m_bAppStateHasNewContent)
    {
        m_bAppStateHasNewContent = false;
    }
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

void HistoryManager::Add(Command* c)
{
    bool bRedosChanged = false;

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
    assert(!m_appState->isObsolete());
    SizeUndos();
    // TODO HistoryManager::Add : after SizeUndos, find an algo to recompute m_appState that could have been invalidated

    observable().Notify(Event::UNDOS_CHANGED);

    if (bRedosChanged)
    {
        observable().Notify(Event::REDOS_CHANGED);
    }
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
}

void HistoryManager::Redo()
{
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
