#include "referentiable.manager.h"
#include "history.manager.h"
#include "os.log.h"

namespace imajuscule
{
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


