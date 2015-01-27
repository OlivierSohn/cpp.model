#include "command.h"
#include "history.manager.h"
#include "os.log.h"
#include <iostream>
#include <cassert>

using namespace imajuscule;

HistoryManager * HistoryManager::g_instance = NULL;

HistoryManager::HistoryManager():
m_stacksCapacity(-1)// unsigned -> maximum capacity
{}
HistoryManager::~HistoryManager()
{
    EmptyStacks();
}

void HistoryManager::EmptyStacks()
{
    EmptyRedos();
    EmptyUndos();
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
}

HistoryManager * HistoryManager::getGlobalInstance()
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
            EmptyRedos();
            m_undos.push_back(c);
            SizeUndos();
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
    if ( size > m_stacksCapacity)
    {
        unsigned int nElementsRemoved = (size - m_stacksCapacity);
        UndoStack::iterator it = m_undos.begin();
        for (unsigned int i = 0; i < nElementsRemoved; i++, ++it)
        {
            delete *it;
        }

        it = m_undos.begin();
        m_undos.erase(it, it + nElementsRemoved);
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
    if (!m_undos.empty())
    {
        Command * c = m_undos.back();
        if (c)
        {
            Command::State s = c->getState();
            if ((s == Command::EXECUTED) || (s == Command::REDO))
            {
                m_undos.pop_back();
                m_redos.push_back(c);
                c->Undo();
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
    else
    {
        std::cout << "\a";
    }
}
void HistoryManager::Redo()
{
    if (!m_redos.empty())
    {
        Command * c = m_redos.back();
        if (c)
        {
            Command::State s = c->getState();
            if (s == Command::UNDO)
            {
                m_redos.pop_back();
                m_undos.push_back(c);
                c->Redo();
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
    else
    {
        std::cout << "\a";
    }
}
