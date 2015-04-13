#include "undoable.h"
#include "os.log.h"
#include "history.manager.h"

using namespace imajuscule;

Undoable::Undoable():
m_obsolete(false)
{}
Undoable::~Undoable()
{
    auto it = m_undoables.begin();
    auto end = m_undoables.end();
    for (; it != end; ++it)
    {
        delete *it;
    }
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

void Undoable::traverseForwardRecurse(Undoables & v) const
{
    for(auto u : m_undoables)
    {
        v.push_back(u);
        
        Undoables v2;
        u->traverseForwardRecurse(v2);
        v.insert(v.end(), v2.begin(), v2.end());
    }
}

void Undoable::Add(Undoable*u)
{
    if(m_curSubElts.empty())
        m_undoables.push_back(u);
    else
        m_curSubElts.top()->Add(u);
}

void Undoable::StartSubElement()
{
    UndoGroup* ug = new UndoGroup();

    Add(ug);
    
    m_curSubElts.push(ug);
}

void Undoable::EndSubElement()
{
    if_A(!m_curSubElts.empty())
    {
        m_curSubElts.pop();
    }
}

bool Undoable::contains(Undoable * u)
{
    bool bRet = false;
    
    if(u)
    {
        Undoables v;
        traverseForwardRecurse(v);
        for(auto e:v)
        {
            if(e == u)
            {
                bRet = true;
                break;
            }
        }
    }
    
    return bRet;
}


