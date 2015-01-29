#include "command.h"
#include "history.manager.h"
#include "os.log.h"
#include <iostream>
#include <cassert>
#include <algorithm>

using namespace imajuscule;

HistoryManager * HistoryManager::g_instance = NULL;

HistoryManager::HistoryManager():
m_stacksCapacity(-1)// unsigned -> maximum capacity
, m_observable(Observable<Event>::instantiate())
{}

HistoryManager::~HistoryManager()
{
    EmptyStacks();

    m_observable->deinstantiate();
}

auto HistoryManager::observable()->Observable<Event> &
{
    return *m_observable;
}

void HistoryManager::EmptyStacks()
{
    EmptyRedos();
    EmptyUndos();

    observable().Notify(Event::UNDOS_CHANGED);
}

void HistoryManager::EmptyRedos()
{
    if (!m_redos.empty())
    {
        RedoStack::iterator it = m_redos.begin();
        RedoStack::iterator end = m_redos.end();
        for (; it != end; ++it)
        {
            delete *it;
        }

        m_redos.clear();

        observable().Notify(Event::REDOS_CHANGED);
    }
}

void HistoryManager::EmptyUndos()
{
    UndoStack::iterator it = m_undos.begin();
    UndoStack::iterator end = m_undos.end();
    for (; it != end; ++it)
    {
        delete *it;
    }

    m_undos.clear();

    // don't notify "undos changed", it's done by the caller
}

HistoryManager * HistoryManager::getInstance()
{
    if (!g_instance)
        g_instance = new HistoryManager();

    return g_instance;
}

void HistoryManager::setStackCapacity(unsigned int size)
{
    m_stacksCapacity = size;
    SizeUndos();
}

void HistoryManager::Add(Command* c)
{
    if (c)
    {
        Command::State s = c->getState();
        if (s == Command::EXECUTED)
        {
            if (!c->isObsolete())
            {
                EmptyRedos();
                m_undos.push_back(c);
                SizeUndos();

                observable().Notify(Event::UNDOS_CHANGED);
            }
            else
            {
                LG(ERR, "HistoryManager::Add : Command is obsolete");
                assert(0);
            }
        }
        else
        {
            LG(ERR, "HistoryManager::Add : Command in state %s", Command::StateToString(s));
            assert(0);
        }
    }
    else
    {
        LG(ERR, "HistoryManager::Add : NULL Command");
        assert(0);
    }
}

void HistoryManager::SizeUndos()
{
    unsigned int size = m_undos.size();
    if (size > m_stacksCapacity)
    {
        unsigned int count = 0;
        for (auto it = m_undos.begin(); it != m_undos.end();)
        {
            if ((*it)->isObsolete())
            {
                delete *it;
                count++;
                it = m_undos.erase(it);
            }
            else
                ++it;
        }

        LG(INFO, "HistoryManager::SizeUndos %u out of %u commands removed because obsolete", count, size);

        size = m_undos.size();
        if (size > m_stacksCapacity)
        {
            unsigned int nElementsRemoved = (size - m_stacksCapacity);
            UndoStack::iterator it = m_undos.begin();
            for (unsigned int i = 0; i < nElementsRemoved; i++, ++it)
            {
                delete *it;
            }

            it = m_undos.begin();
            m_undos.erase(it, it + nElementsRemoved);

            LG(INFO, "HistoryManager::SizeUndos %u first elements removed", nElementsRemoved);
        }
    }
}

unsigned int HistoryManager::CountUndos()
{
    return m_undos.size();
}
unsigned int HistoryManager::CountRedos()
{
    return m_redos.size();
}
void HistoryManager::Undo()
{
    bool bDone = false;
    bool bUndosChanged = false;
    bool bRedosChanged = false;

    while (!m_undos.empty())
    {
        Command * c = m_undos.back();
        if (c)
        {
            if (c->isObsolete())
            {
                m_undos.pop_back();
                bUndosChanged = true;
                delete c;
                continue;
            }

            Command::State s = c->getState();
            if ((s == Command::EXECUTED) || (s == Command::REDO))
            {
                m_undos.pop_back();
                bUndosChanged = true;
                m_redos.push_back(c);
                bRedosChanged = true;
                c->Undo();
                bDone = true;
                break;
            }
            else
            {
                LG(ERR, "HistoryManager::Undo : Command in state %s", Command::StateToString(s));
                assert(0);
            }
        }
        else
        {
            LG(ERR, "HistoryManager::Undo : NULL Command");
            assert(0);
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
    bool bUndosChanged = false;
    bool bRedosChanged = false;
    bool bDone = false;

    while (!m_redos.empty())
    {
        Command * c = m_redos.back();
        if (c)
        {
            if (c->isObsolete())
            {
                m_redos.pop_back();
                bRedosChanged = true;
                delete c;
                continue;
            }

            Command::State s = c->getState();
            if (s == Command::UNDO)
            {
                m_redos.pop_back();
                bRedosChanged = true;
                m_undos.push_back(c);
                bUndosChanged = true;
                c->Redo();
                bDone = true;
                break;
            }
            else
            {
                LG(ERR, "HistoryManager::Redo : Command in state %s", Command::StateToString(s));
                assert(0);
            }
        }
        else
        {
            LG(ERR, "HistoryManager::Redo : NULL Command");
            assert(0);
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

unsigned int HistoryManager::traverseUndos(UndoStack::const_iterator& begin, UndoStack::const_iterator& end)
{
    begin = m_undos.begin();
    end = m_undos.end();

    return m_undos.size();
}
unsigned int HistoryManager::traverseRedos(RedoStack::const_reverse_iterator& begin, RedoStack::const_reverse_iterator& end)
{
    begin = m_redos.rbegin();
    end = m_redos.rend();

    return m_redos.size();
}
