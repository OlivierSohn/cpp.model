
namespace imajuscule
{
template <class InnerCmdType>
bool Command::ReadyFor(const data & Now, const data & Then, Referentiable * pRef)
{
    if ( isObsolete() ) {
        return false;
    }
    
    if(auto * i = dynamic_cast<InnerCmdType*>(this))
    {
        if ((nullptr == pRef) || (pRef == getObject()))
        {
            switch (getState())
            {
            case State::NOT_EXECUTED:
                Assert(!"found an unexecuted inner command");
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
                Assert(0);
                break;
            }
        }
    }

    return false;
}
}


