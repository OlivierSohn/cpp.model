#include "undoable.h"
#include "os.log.h"

using namespace imajuscule;

Undoable::Undoable():
m_state(NOT_EXECUTED)
, m_obsolete(false)
{}
Undoable::~Undoable()
{}

void Undoable::setIsObsolete()
{
    m_obsolete = true;
}

bool Undoable::isObsolete() const
{
    return m_obsolete;
}

auto Undoable::getState() const -> State
{
    return m_state;
}

void Undoable::setState(State state)
{
    m_state = state;
}

bool Undoable::validStateToExecute() const
{
    return (m_state == NOT_EXECUTED);
}
bool Undoable::validStateToUndo() const
{
    return ((m_state == EXECUTED) || (m_state == REDONE));
}
bool Undoable::validStateToRedo() const
{
    return (m_state == UNDONE);
}

const char * Undoable::StateToString(State s)
{
    switch (s)
    {
    case NOT_EXECUTED:
        return "NOT_EXECUTED";
    case EXECUTED:
        return "EXECUTED";
    case UNDONE:
        return "UNDONE";
    case REDONE:
        return "REDONE";
    default:
        return "UNKNOWN";
    }
}

