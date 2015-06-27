#include "referentiable.manager.h"
#include "history.manager.h"
#include "os.log.h"

namespace imajuscule
{
template <class InnerCmdType>
bool Command::ExecuteFromInnerCommand( const data & dataBefore, const data & dataAfter, Referentiable*pRef, const resFunc * pResFunc)
{
    bool bDone = false;
    HistoryManager * h = HistoryManager::getInstance();

    if (Command * c = h->CurrentCommand())
    {
        ExecType t;
        if (h->IsUndoingOrRedoing(t))
        {
            if (c)
            {
                if (!(bDone = c->ExecFromInnerCommand<InnerCmdType>(dataBefore, dataAfter, pRef, pResFunc)))
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
bool Command::ExecFromInnerCommand(const data & dataBefore, const data & dataAfter, Referentiable * pRef, const resFunc * pResFunc)
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
auto Command::ListInnerCommandsReadyFor(const data & dataBefore, const data & dataAfter, Referentiable*pRef, const resFunc * pResFunc)->std::vector < CommandExec >
{
    std::vector < CommandExec > v;

    Undoables::iterator itG, endG;
    traverseForward(itG, endG);
    for (; itG != endG; ++itG)
    {
        Undoable * u = *itG;
        
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

template <class InnerCmdType>
bool Command::ReadyFor(const data & Now, const data & Then, Referentiable * pRef)
{
    bool bRet = false;

    if (isObsolete())
        goto end;

    if (InnerCmdType * i = dynamic_cast<InnerCmdType*>(this))
    {
        if ((NULL == pRef) || (pRef == getObject()))
        {
            switch (getState())
            {
            case State::NOT_EXECUTED:
                A(!"found an unexecuted inner command");
                goto end;
                break;
            case State::UNDONE:
                    if ((Now == *Before()) && (Then == *After()))
                        bRet = true;
                break;

            case State::EXECUTED:
            case State::REDONE:
                    if ((Now == *After()) && (Then == *Before()))
                        bRet = true;
                break;
            default:
                LG(ERR, "ParamChangeFormulaCmd::doExecuteFromInnerCmd : unhandled state %d", getState());
                A(0);
                goto end;
                break;
            }
        }
    }

end:
    return bRet;
}
}


