#include "referentiable.manager.h"
#include "command.h"
#include "history.manager.h"
#include "os.log.h"

using namespace imajuscule;

Command::Command(data * pDataBefore, data * pDataAfter, Referentiable * r, Observable<ObsolescenceEvent> * o) :
Undoable()
, m_state(NOT_EXECUTED)
, m_pAfter(pDataAfter)
, m_pBefore(pDataBefore)
, m_obsolescenceObservable(o)
, m_manager(r?(r->getManager()):NULL)
, m_guid(r?r->guid():std::string())
, m_observable(Observable<Event, const CommandResult *>::instantiate())
{
    if (m_obsolescenceObservable)
        m_reg.push_back(m_obsolescenceObservable->Register(IS_OBSOLETE, std::bind(&Command::onObsolete, this)));
}
Command::~Command()
{
    if (m_pAfter)
        delete m_pAfter;
    if (m_pBefore)
        delete m_pBefore;

    if (m_obsolescenceObservable && !isObsolete())
        m_obsolescenceObservable->Remove(m_reg);

    m_observable->deinstantiate();
}

auto Command::observable()->Observable < Event, const CommandResult * > &
{
    return *m_observable;
}

auto Command::Before() const -> data *
{
    return m_pBefore;
}
auto Command::After() const -> data *
{
    return m_pAfter;
}

void Command::onObsolete()
{
    // on purpose, obsolescence doesn't propagate to inner commands

    if (isObsolete())
    {
        A(!"design error : called at least twice");
    }
    else
    {
        setIsObsolete();
        A(m_obsolescenceObservable);
        if (m_obsolescenceObservable)
        {
            m_obsolescenceObservable->Remove(m_reg);
            m_obsolescenceObservable = NULL;
        }
    }
}

bool Command::Execute()
{
    HistoryManager* h = HistoryManager::getInstance();
    h->PushCurrentCommand(this);

    A(validStateToExecute());

    A(m_undoables.empty());

    bool bRelevant = doExecute();

    setState(EXECUTED);

    // Pop before adding to history, else it will be added as inner command of itself
    h->PopCurrentCommand(this);

    // don't log in history the commands that had no effect (example : backspace when edit location is at begin of input)
    if (bRelevant)
        h->Add(this);
    else
        delete this;

    return bRelevant;
}

bool Command::Undo()
{
    bool bRelevant = false;

    if (validStateToUndo())
    {
        HistoryManager* h = HistoryManager::getInstance();
        h->PushCurrentCommand(this);
        
        // placed after subcommands loop for scenario "new animation / UNDO"
        // But we shouldn't run subcommands before, it breaks the logic (eg deinstantiates stuff too early) so I put it back here
        bRelevant = doUndo();
        
        Undoables::reverse_iterator it = m_undoables.rbegin();
        Undoables::reverse_iterator end = m_undoables.rend();
        for (;it!=end;++it)
        {
            if ((*it)->Undo())
                bRelevant = true;
        }

        setState(UNDONE);

        h->PopCurrentCommand(this);
    }

    return bRelevant;
}

bool Command::Redo()
{
    bool bRelevant = false;

    if (validStateToRedo())
    {
        HistoryManager* h = HistoryManager::getInstance();
        h->PushCurrentCommand(this);

        bRelevant = doRedo();

        for (auto g : m_undoables)
        {
            if (g->Redo())
                bRelevant = true;
        }

        setState(REDONE);

        h->PopCurrentCommand(this);
    }

    return bRelevant;
}

bool Command::Undo(Undoable * limit, bool bStrict, bool & bFoundLimit)
{
    if (this == limit)
        bFoundLimit = true;

    return Undo();
}
bool Command::Redo(Undoable * limit, bool bStrict, bool & bFoundLimit)
{
    if (this == limit)
        bFoundLimit = true;
    
    return Redo();
}

void Command::getExtendedDescription(std::string & desc)
{
    size_t nInner = m_undoables.size();
    if (nInner > 0)
    {
        desc.append("(+");
        desc.append(std::to_string(nInner));
        desc.append(")");
    }
    std::string descCmd;
    getDescription(descCmd);
    desc.append(descCmd);
}

void Command::getDescription(std::string & desc)
{
    if (Referentiable * p = getObject())
        desc.append(p->sessionName());
    else if (m_manager)
        desc.append(std::string("-"));

    if (!desc.empty())
        desc.append(std::string(" : "));
    
    std::string sentence;
    getSentenceDescription(sentence);
    desc.append(sentence);

    switch (getState())
    {
    case NOT_EXECUTED:
        desc.append("Waiting for command execution");
        break;
    default:
        unsigned size = desc.size();
        desc.append(Before()->getDesc());
        if (size != desc.size())
            desc.append(std::string(" -> "));
        desc.append(After()->getDesc());
        break;
    }
}
Command::CommandExec::CommandExec(UndoGroup *g, Command* c, ExecType t, const resFunc *f) :
m_group(g)
, m_command(c)
, m_type(t)
, m_pResFunc(f)
{
    A(m_command);
    A(m_group);
    A(t != ExecType::NONE);
}

Command::CommandExec::~CommandExec()
{}

Command::data::data()
{}
Command::data::~data()
{}

bool Command::data::operator == (const data&d) const
{
    return !(operator!=(d));
}

Referentiable * Command::getObject() const
{
    if (m_manager)
        return m_manager->findByGuid(m_guid);
    else
        return NULL;
}

bool Command::doExecute()
{
    return doExecute(*After());
}

bool Command::doUndo()
{
    return doExecute(*Before());
}

bool Command::doRedo()
{
    return doExecute(*After());
}

bool Command::ExecuteFromInnerCommand(const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable*pRef, const resFunc * pResFunc)
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
                if (!(bDone = c->ExecFromInnerCommand(t, commandType, dataBefore, dataAfter, pRef, pResFunc)))
                {
                    // since we have a current command and are undoing or redoing, the inner command should be there
                    A(!"corresponding inner command not found");
                }
            }
        }
    }

    return bDone;
}

bool Command::ExecFromInnerCommand(ExecType t, const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable * pRef, const resFunc * pResFunc)
{
    bool bDone = false;

    auto v = ListInnerCommandsReadyFor(t, commandType, dataBefore, dataAfter, pRef, pResFunc);
    if (!v.empty())
    {
        if(t == ExecType::UNDO)
            bDone = v.back().Run();
        else
            bDone = v.front().Run();
    }
    return bDone;
}

bool Command::CommandExec::Run()
{
    bool bDone = false;
    
    if_A(m_command)
    {
        if_A(!m_command->isObsolete()) // else the command will be deleted by the group
        {
            FunctionInfo<Event> reg;
            if (m_pResFunc)
                reg = m_command->observable().Register(Event::RESULT, *m_pResFunc);
            
            switch (m_type)
            {
                case ExecType::UNDO:
                    if(m_group)
                        m_group->UndoUntil(m_command);
                    else
                        m_command->Undo();
                    A(m_command->getState() == UNDONE);
                    bDone = true;
                    break;
                case ExecType::REDO:
                    if(m_group)
                        m_group->RedoUntil(m_command);
                    else
                        m_command->Redo();
                    A(m_command->getState() == REDONE);
                    bDone = true;
                    break;
                default:
                    A(!"unknown type");
                    break;
            }
            
            if (m_pResFunc)
                m_command->observable().Remove(reg);
        }
    }
    

    return bDone;
}
auto Command::ListInnerCommandsReadyFor(ExecType t, const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable*pRef, const resFunc * pResFunc)->std::vector < CommandExec >
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
                    if (c2->ReadyFor(t, commandType, dataBefore, dataAfter, pRef))
                    {
                        v.emplace_back(g, c2, t, pResFunc);
                    }
                }
            }
        }
        else if(Command * c = dynamic_cast<Command*>(u))
        {
            if (c->ReadyFor(t, commandType, dataBefore, dataAfter, pRef))
            {
                v.emplace_back((UndoGroup*)NULL, c, t, pResFunc);
            }
        }
        else
            A(0);
    }

    return v;
}

bool Command::ReadyFor(ExecType t, const std::type_info & commandType, const data & Now, const data & Then, Referentiable * pRef)
{
    bool bRet = false;

    if (isObsolete())
        goto end;

    if (typeid(*this) == commandType)
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
                if (t != ExecType::REDO)
                    goto end;
                break;

            case State::EXECUTED:
            case State::REDONE:
                if (t != ExecType::UNDO)
                    goto end;
                break;
            default:
                LG(ERR, "ParamChangeFormulaCmd::doExecuteFromInnerCmd : unhandled state %d", getState());
                A(0);
                goto end;
                break;
            }

            if (t == ExecType::REDO)
            {
                if ((Now == *Before()) && (Then == *After()))
                {
                    bRet = true;
                }
            }
            else
            {
                A(t == ExecType::UNDO);
                if ((Now == *After()) && (Then == *Before()))
                {
                    bRet = true;
                }
            }
        }
    }

end:
    return bRet;
}

auto Command::getState() const -> State
{
    return m_state;
}

void Command::setState(State state)
{
    m_state = state;
}

bool Command::validStateToExecute() const
{
    return (m_state == NOT_EXECUTED);
}
bool Command::validStateToUndo() const
{
    return ((m_state == EXECUTED) || (m_state == REDONE));
}
bool Command::validStateToRedo() const
{
    return (m_state == UNDONE);
}

Command::CommandResult::CommandResult(bool bSuccess) :
m_bInitialized(true)
, m_success(bSuccess)
{}
Command::CommandResult::CommandResult() :
m_bInitialized (false)
{}
Command::CommandResult::~CommandResult()
{}

bool Command::CommandResult::initialized() const
{
    return m_bInitialized;
}
bool Command::CommandResult::Success() const
{
    A(initialized());
    return m_success;
}


const char * Command::StateToString(State s)
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


