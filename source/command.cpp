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

    for (auto c : m_innerCommands) delete c;
}

void Command::onObsolete()
{
    // on purpose, obsolescence doesn't propagate to inner commands

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

bool Command::isObsolete() const
{
    return m_obsolete;
}

auto Command::getState() const -> State
{
    return m_state;
}

bool Command::Execute()
{
    HistoryManager* h = HistoryManager::getInstance();
    h->PushCurrentCommand(this);

    assert(m_state == NOT_EXECUTED);

    assert(m_innerCommands.empty());

    bool bAddToHistory = doExecute();

    m_state = EXECUTED;

    // Pop before adding to history, else it will be added as inner command of itself
    h->PopCurrentCommand(this);

    // don't log in history the commands that had no effect (example : backspace when edit location is at begin of input)
    if (bAddToHistory)
        HistoryManager::getInstance()->Add(this);
    else
        delete this;

    return bAddToHistory;
}

void Command::traverseInnerCommands(Commands::iterator & begin, Commands::iterator & end)
{
    begin = m_innerCommands.begin();
    end = m_innerCommands.end();
}

bool Command::validStateToUndo() const
{
    return ((m_state == EXECUTED) || (m_state == REDO));
}
bool Command::validStateToRedo() const
{
    return (m_state == UNDO);
}
void Command::Undo()
{
    if (validStateToUndo())
    {
        HistoryManager* h = HistoryManager::getInstance();
        h->PushCurrentCommand(this);

        doUndo();

        for (auto c : m_innerCommands)
        {
            if (c->isObsolete())
                continue;

            switch (c->getState())
            {
            case UNDO:
                // doUndo of this command has triggered UNDO of inner command, so skip it
                continue;
                break;

            default:
                // doUndo of this command has NOT triggered UNDO of inner command, so do it
                c->doUndo();
                break;
            }
        }

        m_state = UNDO;
        h->PopCurrentCommand(this);
    }
    else
    {
        LG(ERR, "Command::Undo : state is %d", m_state);
        assert(0);
    }
}

void Command::Redo()
{
    if (validStateToRedo())
    {
        HistoryManager* h = HistoryManager::getInstance();
        h->PushCurrentCommand(this);

        doRedo();

        for (auto c : m_innerCommands)
        {
            if (c->isObsolete())
                continue;

            switch (c->getState())
            {
            case REDO:
                // doRedo of this command has triggered REDO of inner command, so skip it
                continue;
                break;

            default:
                // doRedo of this command has NOT triggered REDO of inner command, so do it
                c->doRedo();
                break;
            }
        }

        m_state = REDO;
        h->PopCurrentCommand(this);
    }
    else
    {
        LG(ERR, "Command::Redo : state is %d", m_state);
        assert(0);
    }
}

void Command::addInnerCommand(Command*c)
{
    if(c)
        m_innerCommands.push_back(c);
    else
    {
        LG(ERR, "Command::addInnerCommand : NULL param");
        assert(0);
    }
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
