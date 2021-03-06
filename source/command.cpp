
using namespace imajuscule;

Command::Command(data * pDataBefore, data * pDataAfter, Referentiable * r, Observable<ObsolescenceEvent> * o) :
Undoable()
, m_state(NOT_EXECUTED)
, m_pAfter(pDataAfter)
, m_pBefore(pDataBefore)
, m_obsolescenceObservable(o)
, m_manager(r?(r->getManager()):nullptr)
, m_guid(r?r->guid():std::string())
{
    if (m_obsolescenceObservable) {
        m_reg.push_back(m_obsolescenceObservable->Register(IS_OBSOLETE, std::bind(&Command::onObsolete, this)));
    }
}

Command::~Command()
{
    if (m_obsolescenceObservable && !isObsolete()) {
        m_obsolescenceObservable->Remove(m_reg);
    }
}

auto Command::Before() const -> data * {
    return m_pBefore.get();
}

auto Command::After() const -> data * {
    return m_pAfter.get();
}

void Command::onObsolete() {
    // on purpose, obsolescence doesn't propagate to inner commands

    if (unlikely(isObsolete())) {
        Assert(!"design error : called at least twice");
    }
    else {
        setIsObsolete();
        Assert(m_obsolescenceObservable);
        if (m_obsolescenceObservable)
        {
            m_obsolescenceObservable->Remove(m_reg);
            m_obsolescenceObservable = nullptr;
        }
    }
}

bool Command::Execute() {
    auto* h = HistoryManager::getInstance();
    h->PushCurrentCommand(this);

    Assert(validStateToExecute());

    Assert(m_undoables.empty());

    bool bRelevant = doExecute();

    setState(EXECUTED);

    // Pop before adding to history, else it will be added as inner command of itself
    h->PopCurrentCommand(this);

    // don't log in history the commands that had no effect (example : backspace when edit location is at begin of input)
    if (bRelevant) {
        h->Add(this);
    }
    else {
        delete this;
    }

    return bRelevant;
}

bool Command::Undo()
{
    if (!validStateToUndo()) {
        return false;
    }
    
    HistoryManager* h = HistoryManager::getInstance();
    h->PushCurrentCommand(this);
    
    // placed after subcommands loop for scenario "new animation / UNDO"
    // But we shouldn't run subcommands before, it breaks the logic (eg deinstantiates stuff too early) so I put it back here
    auto bRelevant = doUndo();
    
    auto it = m_undoables.rbegin();
    auto end = m_undoables.rend();
    for (;it!=end;++it) {
        if ((*it)->Undo()) {
            bRelevant = true;
        }
    }
    
    setState(UNDONE);
    
    h->PopCurrentCommand(this);
    
    return bRelevant;
}

bool Command::Redo()
{
    if (!validStateToRedo()) {
        return false;
    }
    HistoryManager* h = HistoryManager::getInstance();
    h->PushCurrentCommand(this);
    
    auto bRelevant = doRedo();
    
    for (auto const &g : m_undoables) {
        if (g->Redo()) {
            bRelevant = true;
        }
    }
    
    setState(REDONE);
    
    h->PopCurrentCommand(this);

    return bRelevant;
}

bool Command::Undo(Undoable * limit, bool bStrict, bool & bFoundLimit)
{
    if (this == limit) {
        bFoundLimit = true;
    }
    return Undo();
}
bool Command::Redo(Undoable * limit, bool bStrict, bool & bFoundLimit)
{
    if (this == limit) {
        bFoundLimit = true;
    }
    return Redo();
}

void Command::getDescription(std::string & desc) const
{
    if (auto p = getObject()) {
        desc.append(p->sessionName());
    }
    else if (m_manager) {
        desc.append(std::string("-"));
    }

    if (!desc.empty()) {
        desc.append(std::string(" : "));
    }
    
    std::string sentence;
    getSentenceDescription(sentence);
    desc.append(sentence);
    
    switch (getState())
    {
        case NOT_EXECUTED:
            desc.append("Waiting for command execution");
            break;
        default:
            auto size = desc.size();
            desc.append(Before()->getDesc());
            if (size != desc.size()) {
                desc.append(std::string(" -> "));
            }
            desc.append(After()->getDesc());
            break;
    }
}
Undoable::CommandExec::CommandExec(UndoGroup *g, Command* c, const resFunc *f) :
m_group(g)
, m_command(c)
, m_pResFunc(f)
{
    Assert(m_command);
}


bool Command::data::operator == (const data&d) const
{
    return !(operator!=(d));
}

Referentiable * Command::getObject() const
{
    if (!m_manager) {
        return {};
    }
    return m_manager->findByGuid(m_guid);
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

bool Undoable::CommandExec::Run()
{
    Assert(m_command);
    Assert(!m_command->isObsolete()); // else the command will be deleted by the group
    FunctionInfo<Event> reg;
    if (m_pResFunc) {
        reg = m_command->observable().Register(Event::RESULT, *m_pResFunc);
    }
    
    switch (m_command->getState())
    {
        case Command::State::EXECUTED:
        case Command::State::REDONE:
            if(m_group) {
                m_group->UndoUntil(m_command);
            }
            else {
                m_command->Undo();
            }
            Assert(m_command->getState() == UNDONE);
            break;
        case Command::State::UNDONE:
            if(m_group) {
                m_group->RedoUntil(m_command);
            }
            else {
                m_command->Redo();
            }
            Assert(m_command->getState() == REDONE);
            break;
        default:
            Assert(!"unhandled type");
            break;
    }
    
    if (m_pResFunc) {
        m_command->observable().Remove(reg);
    }

    return true;
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

bool Command::validStateToUndo() const {
    return ((m_state == EXECUTED) || (m_state == REDONE));
}

bool Command::validStateToRedo() const {
    return (m_state == UNDONE);
}

Command::CommandResult::CommandResult(bool bSuccess) :
m_bInitialized(true)
, m_success(bSuccess)
{}

Command::CommandResult::CommandResult() :
m_bInitialized (false)
{}

bool Command::CommandResult::initialized() const {
    return m_bInitialized;
}

bool Command::CommandResult::Success() const {
    Assert(initialized());
    return m_success;
}



