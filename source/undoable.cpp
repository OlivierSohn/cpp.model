#include "undoable.h"
#include "os.log.h"
#include "history.manager.h"

using namespace imajuscule;

Undoable::Undoable():
m_obsolete(false)
, m_observable(Observable<Event, const CommandResult *>::instantiate())
{}

Undoable::~Undoable()
{
    m_observable->deinstantiate();
}

auto Undoable::observable()->Observable < Event, const CommandResult * > &
{
    return *m_observable;
}

void Undoable::setIsObsolete()
{
    m_obsolete = true;
}

bool Undoable::isObsolete() const
{
    return m_obsolete;
}
void Undoable::traverseForward(Undoables::iterator & it, Undoables::iterator & end) const
{
    it = m_undoables.begin();
    end = m_undoables.end();
}

void Undoable::traverseForwardRecurse(std::vector<Undoable*> & v) const
{
    for(auto const & u : m_undoables)
    {
        v.push_back(u.get());
        
        std::vector<Undoable*> v2;
        u->traverseForwardRecurse(v2);
        v.insert(v.end(), v2.begin(), v2.end());
    }
}

void Undoable::Add(Undoable*u)
{
    if(m_curSubElts.empty()) {
        m_undoables.emplace_back(std::unique_ptr<Undoable>(u));
    }
    else {
        m_curSubElts.top()->Add(u);
    }
}

void Undoable::StartSubElement()
{
    auto * ug = new UndoGroup();
    Add(ug);
    m_curSubElts.push(ug);
}

bool Undoable::EndSubElement()
{
    if(m_curSubElts.empty()) {
        return false;
    }
    m_curSubElts.pop();
    return true;
}
/*
bool Undoable::contains(Undoable * u)
{
    if(!u) {
        return false;
    }
    std::vector<Undoable*> v;
    traverseForwardRecurse(v);
    for(auto e:v) {
        if(e == u) {
            return true;
        }
    }
}*/

void Undoable::getNRExtendedDescription(std::string & desc) const
{
    std::string descCmd;
    getDescription(descCmd);
    desc.append(descCmd);
}
void Undoable::getExtendedDescription(std::string & desc, size_t offset) const
{
    size_t nInner = m_undoables.size();
    if (nInner > 0)
    {
        desc.append("(+");
        desc.append(std::to_string(nInner));
        desc.append(")");
    }

    getNRExtendedDescription(desc);
    
    for(auto const & u:m_undoables)
    {
        desc.append("\n");
        desc.append(offset + 4, ' ');
        std::string desc2;
        u->getExtendedDescription(desc2, offset+4);
        desc.append(desc2);
    }
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
