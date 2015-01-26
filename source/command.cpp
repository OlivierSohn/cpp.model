#include "command.h"
#include "history.manager.h"
#include <cassert>
using namespace imajuscule;

Command::Command(HistoryManager * hm):
m_state(NOT_EXECUTED),
m_history(hm ? hm : HistoryManager::getGlobalInstance())
{}
Command::~Command()
{}

HistoryManager * Command::getHistoryManager()
{
    return m_history;
}

auto Command::getState() -> State
{
    return m_state;
}

void Command::Execute()
{
    assert(m_state == NOT_EXECUTED);

    bool bAddToHistory = doExecute();

    m_state = EXECUTED;

    // don't log in history the commands that had no effect (example : backspace when edit location is at begin of input)
    if (bAddToHistory)
        getHistoryManager()->Add(this);
    else
        delete this;
}

void Command::Undo()
{
    assert((m_state == EXECUTED) || (m_state == REDO));

    doUndo();

    m_state = UNDO;
}

void Command::Redo()
{
    assert(m_state == UNDO);

    doRedo();

    m_state = REDO;
}

const char * Command::StateToString(State s)
{
    switch (s)
    {
    case NOT_EXECUTED:
        return "NOT_EXECUTED";
    case EXECUTED:
        return "EXECUTED";
    case UNDO:
        return "UNDO";
    case REDO:
        return "REDO";
    default:
        return "UNKNOWN";
    }
}
