#include "command.h"
#include "history.manager.h"
#include <cassert>
#include "os.log.h"

using namespace imajuscule;

Command::Command(Observable<ObsolescenceEvent> * o) :
m_state(NOT_EXECUTED),
m_obsolete(false),
m_obsolescenceObservable(o)
{
    if ( m_obsolescenceObservable )
        m_reg.push_back( m_obsolescenceObservable->Register(IS_OBSOLETE, std::bind(&Command::onObsolete, this)));
}
Command::~Command()
{
    if ( m_obsolescenceObservable && !m_obsolete )
        m_obsolescenceObservable->Remove(m_reg);
}

void Command::onObsolete()
{
    if (m_obsolete)
    {
        LG(ERR, "Command::onObsolete() design error : called at least twice");
        assert(0);
    }
    else
    {
        m_obsolete = true;
        assert(m_obsolescenceObservable);
        if (m_obsolescenceObservable)
        {
            m_obsolescenceObservable->Remove(m_reg);
            m_obsolescenceObservable = NULL;
        }
    }
}

bool Command::isObsolete()
{
    return m_obsolete;
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
        HistoryManager::getInstance()->Add(this);
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
