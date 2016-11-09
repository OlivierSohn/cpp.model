#pragma once

#include "os.log.h"

namespace imajuscule
{
template <class InnerCmdType>
bool Command::ReadyFor(const data & Now, const data & Then, Referentiable * pRef)
{
    if ( isObsolete() ) {
        return false;
    }
    
    if (InnerCmdType * i = dynamic_cast<InnerCmdType*>(this))
    {
        if ((nullptr == pRef) || (pRef == getObject()))
        {
            switch (getState())
            {
            case State::NOT_EXECUTED:
                A(!"found an unexecuted inner command");
                break;
            case State::UNDONE:
                    if ((Now == *Before()) && (Then == *After())) {
                        return true;
                    }
                break;

            case State::EXECUTED:
            case State::REDONE:
                    if ((Now == *After()) && (Then == *Before())) {
                        return true;
                    }
                break;
            default:
                LG(ERR, "ParamChangeFormulaCmd::doExecuteFromInnerCmd : unhandled state %d", getState());
                A(0);
                break;
            }
        }
    }

    return false;
}
}


