
#include "history.manager.h"
#include "command.h"

namespace imajuscule
{
template <class InnerCmdType>
bool Undoable::ExecuteFromInnerCommand( const data & dataBefore, const data & dataAfter, Referentiable*pRef, const resFunc * pResFunc)
{
    bool bDone = false;
    HistoryManager * h = HistoryManager::getInstance();

    if (auto * c = h->CurrentCommand())
    {
        ExecutionType t;
        if (h->IsUndoingOrRedoing(t))
        {
            if (c)
            {
                if (unlikely(!(bDone = c->ExecFromInnerCommand<InnerCmdType>(dataBefore, dataAfter, pRef, pResFunc))))
                {
                    // since we have a current command and are undoing or redoing, the inner command should be there
                    A(!"corresponding inner command not found");
                }
            }
        }
    }

    return bDone;
}
    
    template <class InnerCmdType>
    bool Undoable::ExecFromInnerCommand(const data & dataBefore, const data & dataAfter, Referentiable * pRef, const resFunc * pResFunc)
    {
        bool bDone = false;
        
        auto v = ListInnerCommandsReadyFor<InnerCmdType>(dataBefore, dataAfter, pRef, pResFunc);
        if (!v.empty())
        {
            bDone = v.back().Run();
        }
        return bDone;
    }

template <class InnerCmdType>
auto Undoable::ListInnerCommandsReadyFor(const data & dataBefore, const data & dataAfter, Referentiable*pRef, const resFunc * pResFunc)->std::vector < CommandExec >
{
    std::vector < CommandExec > v;

    Undoables::iterator itG, endG;
    traverseForward(itG, endG);
    for (; itG != endG; ++itG)
    {
        Undoable * u = itG->get();
        
        if(UndoGroup * g = dynamic_cast<UndoGroup*>(u))
        {
            std::vector<Undoable*> v2;
            g->traverseForwardRecurse(v2);
            for (auto u2 : v2)
            {
                if( Command * c2 = dynamic_cast<Command*>(u2) )
                {
                    if (c2->ReadyFor<InnerCmdType>(dataBefore, dataAfter, pRef))
                    {
                        v.emplace_back(g, c2, pResFunc);
                    }
                }
            }
        }
        else if(Command * c = dynamic_cast<Command*>(u))
        {
            if (c->ReadyFor<InnerCmdType>(dataBefore, dataAfter, pRef))
            {
                v.emplace_back((UndoGroup*)NULL, c, pResFunc);
            }
        }
        else
            A(0);
    }

    return v;
}
}


