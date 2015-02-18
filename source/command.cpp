#include "referentiable.manager.h"
#include "command.h"
#include "history.manager.h"
#include "os.log.h"

using namespace imajuscule;

Command::Command(data * pDataBefore, data * pDataAfter, Referentiable * r, Observable<ObsolescenceEvent> * o) :
m_state(NOT_EXECUTED)
, m_pAfter(pDataAfter)
, m_pBefore(pDataBefore)
, m_obsolete(false)
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

    if (m_obsolescenceObservable && !m_obsolete)
        m_obsolescenceObservable->Remove(m_reg);

    for (auto c : m_innerCommands) delete c;

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


bool Command::isInnerCommand(Command * command)
{
    bool bRet = false;

    for (auto c : m_innerCommands)
    {
        if (c == command)
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}
void Command::onObsolete()
{
    // on purpose, obsolescence doesn't propagate to inner commands

    if (m_obsolete)
    {
        A(!"design error : called at least twice");
    }
    else
    {
        m_obsolete = true;
        A(m_obsolescenceObservable);
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

    A(m_state == NOT_EXECUTED);

    A(m_innerCommands.empty());

    bool bRelevant = doExecute();

    m_state = EXECUTED;

    // Pop before adding to history, else it will be added as inner command of itself
    h->PopCurrentCommand(this);

    // don't log in history the commands that had no effect (example : backspace when edit location is at begin of input)
    if (bRelevant)
        h->Add(this);
    else
        delete this;

    return bRelevant;
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
            {
                HistoryManager::getInstance()->logObsoleteCommand(c);
                continue;
            }

            switch (c->getState())
            {
            case UNDO:
                // doUndo of this command has triggered UNDO of inner command, so skip it
                continue;
                break;

            default:
                // doUndo of this command has NOT triggered UNDO of inner command, so do it
                c->Undo();
                break;
            }
        }

        m_state = UNDO;
        h->PopCurrentCommand(this);
    }
    else
    {
        LG(ERR, "Command::Undo : state is %d", m_state);
        A(0);
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
            {
                HistoryManager::getInstance()->logObsoleteCommand(c);
                continue;
            }

            switch (c->getState())
            {
            case REDO:
                // doRedo of this command has triggered REDO of inner command, so skip it
                continue;
                break;

            default:
                // doRedo of this command has NOT triggered REDO of inner command, so do it
                c->Redo();
                break;
            }
        }

        m_state = REDO;
        h->PopCurrentCommand(this);
    }
    else
    {
        LG(ERR, "Command::Redo : state is %d", m_state);
        A(0);
    }
}

void Command::addInnerCommand(Command*c)
{
    if_A(c)
        m_innerCommands.push_back(c);
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

void Command::getExtendedDescription(std::string & desc)
{
    unsigned int nInner = m_innerCommands.size();
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
Command::CommandExec::CommandExec(Command* c, Type t) :
m_command(c)
, m_type(t)
{
    A(m_command);
}

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

void Command::doUndo()
{
    doExecute(*Before());
}

void Command::doRedo()
{
    doExecute(*After());
}

bool Command::ExecuteFromInnerCommand(const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable*pRef, const resFunc * pResFunc)
{
    bool bDone = false;
    HistoryManager * h = HistoryManager::getInstance();

    if (Command * c = h->CurrentCommand())
    {
        if (h->IsUndoingOrRedoing())
        {
            if (c)
            {
                if (!(bDone = c->ExecFromInnerCommand(commandType, dataBefore, dataAfter, pRef, pResFunc)))
                {
                    // since we have a current command and are undoing or redoing, the inner command should be there
                    A(!"corresponding inner command not found");
                }
            }
        }
    }

    return bDone;
}

bool Command::ExecFromInnerCommand(const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable * pRef, const resFunc * pResFunc)
{
    bool bDone = false;

    auto v = ListInnerCommandsReadyFor(commandType, dataBefore, dataAfter, pRef );
    if (!v.empty())
    {
        unsigned int size = v.size();
        if (size > 1)
        {
            LG(ERR, "Command::ExecFromInnerCommand(0x%x, %s) : multiple (%d) results", this, (pRef?pRef->sessionName().c_str():"NULL"), size);
            A(0);
        }

        auto & commandExec = v[0];
        Command * c = commandExec.m_command;
        if_A(c)
        {
            FunctionInfo<Event> reg;
            if (pResFunc)
                reg = c->observable().Register(Event::RESULT, *pResFunc);
            
            switch (commandExec.m_type)
            {
            case CommandExec::UNDO:
                c->Undo();
                bDone = true;
                break;
            case CommandExec::REDO:
                c->Redo();
                bDone = true;
                break;
            default:
                A(!"unknown type");
                break;
            }

            if (pResFunc)
                c->observable().Remove(reg);
        }
    }
    return bDone;
}
auto Command::ListInnerCommandsReadyFor(const std::type_info & commandType, const data & dataBefore, const data & dataAfter, Referentiable*pRef)->std::vector < CommandExec >
{
    std::vector < CommandExec > v;

    Commands::iterator it, end;
    traverseInnerCommands(it, end);
    for (; it != end; ++it)
    {
        Command * c = *it;
        if_A (c)
        {
            CommandExec::Type t;
            if (c->ReadyFor(commandType, dataBefore, dataAfter, pRef, t))
            {
                v.emplace_back(c, t);
            }
        }
    }

    return v;
}

bool Command::ReadyFor(const std::type_info & commandType, const data & Now, const data & Then, Referentiable * pRef, CommandExec::Type & t)
{
    bool bRet = false;

    if (typeid(*this) == commandType)
    {
        if ((NULL == pRef) || (pRef == getObject()))
        {
            switch (getState())
            {
            case Command::State::NOT_EXECUTED:
                A(!"found an unexecuted inner command");
                goto end;
                break;
            case Command::State::UNDO:
                t = CommandExec::REDO;
                break;

            case Command::State::EXECUTED:
            case Command::State::REDO:
                t = CommandExec::UNDO;
                break;
            default:
                LG(ERR, "ParamChangeFormulaCmd::doExecuteFromInnerCmd : unhandled state %d", getState());
                A(0);
                goto end;
                break;
            }

            if (t == CommandExec::REDO)
            {
                if ((Now == *Before()) && (Then == *After()))
                {
                    bRet = true;
                }
            }
            else
            {
                A(t == CommandExec::UNDO);
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
