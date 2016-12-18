
using namespace imajuscule;

void HistoryManager::logObsoleteCommand(Command*c)
{
    logCommand(c, "[obsolete] ");
}

void HistoryManager::logCommand(Command*c, const char * pre)
{
    A(c);
    std::string pad;
    pad.insert(pad.end(), m_curCommandStack.size(), ' ');
    std::string desc;
    c->getExtendedDescription(desc, 0);
    LG(INFO, "%s%s%s", pad.c_str(), pre ? pre : "", desc.c_str());
}

void UndoGroup::getDescription(std::string &desc) const
{
    desc.append("group");
}
bool UndoGroup::isObsolete() const
{
    bool bEmpty = true;

    auto it = m_undoables.begin();
    auto end = m_undoables.end();
    while (it != end)
    {
        if ((*it)->isObsolete())
        {
            it = m_undoables.erase(it);
            end = m_undoables.end(); // because vector changed
        }
        else
        {
            bEmpty = false;
            ++it;
        }
    }

    return bEmpty;
}

bool UndoGroup::Execute()
{
    A(0);
    return false;
}

bool UndoGroup::Undo(){ bool bFoundLimit; return Undo(nullptr, false, bFoundLimit); }
bool UndoGroup::UndoUntil(Undoable *limit){ bool bFoundLimit; return Undo(limit, true, bFoundLimit); }
bool UndoGroup::Undo(Undoable *limit, bool bStrict, bool & bFoundLimit)
{
    bool bNotEmpty = false;

    auto it = m_undoables.rbegin();
    auto end = m_undoables.rend();

    bFoundLimit = false;

    while(it != end)
    {
        Undoable * u = it->get();
        A(u);
        if (u->isObsolete())
        {
            it = std::reverse_iterator<Undoables::iterator>(m_undoables.erase((std::next(it)).base()));
            end = m_undoables.rend();
            continue;
        }
        
        bNotEmpty = true;
        
        u->Undo(limit, false, bFoundLimit);
        
        // no need to check return value : a relevant (executed) command can become irrelevant for undo/redo e.g. SetFormula("", "0.")
        // TODO -> should I introduce the notion of undoability and have 2 different commands: ParamInitializeFormula(not undoable) and ParamChangeForula(undoable) ?
        
        if(bFoundLimit)
            break;

        ++it;
    }

    if (bStrict)
    {
        A(bFoundLimit);
    }
    
    return bNotEmpty;
}
bool UndoGroup::Redo(){ bool bFoundLimit; return Redo(nullptr, false, bFoundLimit); }
bool UndoGroup::RedoUntil(Undoable * limit){ bool bFoundLimit; return Redo(limit, true, bFoundLimit); }
bool UndoGroup::Redo(Undoable * limit, bool bStrict, bool & bFoundLimit)
{
    bool bNotEmpty = false;

    auto it = m_undoables.begin();
    auto end = m_undoables.end();
    bFoundLimit = false;

    while (it != end)
    {
        Undoable * u = it->get();
        A(u);
        if (u->isObsolete())
        {
            it = m_undoables.erase(it);
            end = m_undoables.end();
            continue;
        }
        
        bNotEmpty = true;
        
        //            if(u->validStateToRedo())// command can have been undone by a call to RedoUntil
        {
            /*bool bRelevant =*/
            u->Redo(limit, false, bFoundLimit);
            // Assert commented out : a relevant (executed) command can become irrelevant for undo/redo e.g. SetFormula("", "0.")
            // -> should I introduce the notion of undoability?
            //    and have 2 different commands: ParamInitializeFormula(not undoable) and ParamChangeForula(undoable) ?
            //A(bRelevant);
        }
        
        if(bFoundLimit)
            break;
    
        ++it;
    }

    if (bStrict)
    {
        A(bFoundLimit);
    }
    
    return bNotEmpty;
}


HistoryManager * HistoryManager::g_instance = nullptr;

HistoryManager::HistoryManager() :
m_stacksCapacity(-1)// unsigned -> maximum capacity
, m_observable(Observable<Event>::instantiate())
, m_bAppStateHasNewContent(false)
, m_curExecType(ExecutionType::NONE)
, m_iActivated(1)
{
    m_appState = m_groups.rbegin();
}

HistoryManager::~HistoryManager()
{
    reset();

    m_observable->deinstantiate();
}

auto HistoryManager::observable()->Observable<Event> &
{
    return *m_observable;
}

void HistoryManager::PushPause()
{
    A(m_iActivated <= 1);
    m_iActivated --;
}
void HistoryManager::PopPause()
{
    m_iActivated ++;
    A(m_iActivated <= 1);
}
bool HistoryManager::isActive() const
{
    A(m_iActivated <= 1);
    return m_iActivated == 1;
}
void HistoryManager::MakeGroup()
{
    if(nullptr == CurrentCommand() && transactionCount_ == 0)
    {
        if (m_bAppStateHasNewContent)
        {
            m_bAppStateHasNewContent = false;
        }
    }
}
void HistoryManager::StartTransaction(){
    if (auto * cc = CurrentCommand())
        cc->StartSubElement();
    transactionCount_++;
}
void HistoryManager::EndTransaction(){
    transactionCount_--;
    if (auto * cc = CurrentCommand()) {
        cc->EndSubElement();
    }
}

void HistoryManager::reset()
{
    A(m_curExecType == ExecutionType::NONE);
    
    m_curExecType = ExecutionType::NONE;
    
    A(transactionCount_ == 0);
    transactionCount_ = 0;
    
    m_iActivated = 1;
    
    while ( ! m_curCommandStack.empty() ) {
        m_curCommandStack.pop();
    }
    
    m_groups.clear();
    m_appState = m_groups.rbegin();
    m_bAppStateHasNewContent = false;
}

HistoryManager * HistoryManager::getInstance()
{
    return Globals::ptr<HistoryManager>(g_instance);
}

Undoable * HistoryManager::CurrentCommand()
{
    if (m_curCommandStack.empty()) {
        return nullptr;
    }
    return m_curCommandStack.top();
}

void HistoryManager::PushCurrentCommand(Undoable*c)
{
    //logCommand(c);
    m_curCommandStack.push(c);
}
void HistoryManager::PopCurrentCommand(Undoable*c)
{
    A(!m_curCommandStack.empty());
    A(m_curCommandStack.top() == c);
    m_curCommandStack.pop();
}

bool HistoryManager::IsUndoingOrRedoing(ExecutionType & t)
{
    t = m_curExecType;
    return (t != ExecutionType::NONE);
}

void HistoryManager::Add(Undoable* c)
{
    bool bRedosChanged = false;

    if ( unlikely(m_curExecType != ExecutionType::NONE))
    {
        LG(ERR, "HistoryManager::Add : Memory leak : a command was added to history while %s", (m_curExecType==ExecutionType::UNDO)?"undoing":"redoing");
        A(0);
        return;
    }

    if (unlikely(!isActive()))
    {
        // memory leak
        LG(ERR, "HistoryManager::Add : Memory leak : a command was added to history while inactive");
        A(0);
        return;
    }

    if (auto * cmd = CurrentCommand())
    {
        cmd->Add(c);
        return;
    }

    if (!m_bAppStateHasNewContent)
    {
        m_bAppStateHasNewContent = true;
        // erase "redos"
        auto firstRedoIt = m_appState.base();
        auto end = m_groups.end();
        if (firstRedoIt != end)
        {
            m_groups.erase(firstRedoIt, end);
            bRedosChanged = true;
        }

        // new group
        m_groups.emplace_back();
        m_appState = m_groups.rbegin();
    }

    m_appState->Add(c);
    A(!m_appState->isObsolete());
    SizeUndos();
    // TODO HistoryManager::Add : after SizeUndos, find an algo to recompute m_appState that could have been invalidated

    observable().Notify(Event::UNDOS_CHANGED);

    if (bRedosChanged) {
        observable().Notify(Event::REDOS_CHANGED);
    }
}

void HistoryManager::SizeUndos()
{
    unsigned int size = (unsigned int) m_groups.size();
    if (size > m_stacksCapacity)
    {
        unsigned int count = 0;
        auto end = m_groups.end();
        for (auto it = m_groups.begin(); it != end ;)
        {
            if (it->isObsolete())
            {
                count++;
                it = m_groups.erase(it);
                end = m_groups.end();
            }
            else {
                ++it;
            }
        }

        LG(INFO, "HistoryManager::SizeUndos %u out of %u groups removed because empty", count, size);

        size = (unsigned int) m_groups.size();
        if (size > m_stacksCapacity)
        {
            unsigned int nElementsRemoved = (size - m_stacksCapacity);
            UndoGroups::iterator it = m_groups.begin();
            m_groups.erase(it, std::next(it, nElementsRemoved));

            LG(INFO, "HistoryManager::SizeUndos %u first groups removed", nElementsRemoved);
        }
    }
}

void HistoryManager::Undo()
{
    m_curExecType = ExecutionType::UNDO;

    bool bDone = false;
    bool bUndosChanged = false;
    bool bRedosChanged = false;

    while (m_appState != m_groups.rend() /*rend must be recomputed at each loop*/)
    {
        if (!m_appState->Undo()) {
            // remove this empty state and increment
            bUndosChanged = true;

            m_appState = std::reverse_iterator<UndoGroups::iterator>(m_groups.erase(std::next(m_appState).base()));
        }
        else {
            ++m_appState;
            bDone = true;
            bRedosChanged = true;
            bUndosChanged = true;
            break;
        }
    }
    
    if (!bDone) {
        std::cout << "\a";
    }

    if (bUndosChanged) {
        observable().Notify(Event::UNDOS_CHANGED);
    }

    if (bRedosChanged) {
        observable().Notify(Event::REDOS_CHANGED);
    }

    m_curExecType = ExecutionType::NONE;
}

void HistoryManager::Redo()
{
    m_curExecType = ExecutionType::REDO;

    bool bDone = false;
    bool bUndosChanged = false;
    bool bRedosChanged = false;

    while (m_appState != m_groups.rbegin() /*rbegin must be recomputed at each loop*/ )
    {
        --m_appState;

        if (!m_appState->Redo()) {
            // remove this empty state
            bRedosChanged = true;

            m_appState = std::reverse_iterator<UndoGroups::iterator>(m_groups.erase(std::next(m_appState).base()));
        }
        else {
            bDone = true;
            bRedosChanged = true;
            bUndosChanged = true;
            break;
        }
    }

    if (!bDone) {
        std::cout << "\a";
    }

    if (bUndosChanged) {
        observable().Notify(Event::UNDOS_CHANGED);
    }

    if (bRedosChanged) {
        observable().Notify(Event::REDOS_CHANGED);
    }

    m_curExecType = ExecutionType::NONE;
}

void HistoryManager::traverseUndos(UndoGroups::const_iterator& begin, UndoGroups::const_iterator& end) const
{
    begin = m_groups.begin();
    end = m_appState.base();
}
void HistoryManager::traverseRedos(UndoGroups::const_iterator& begin, UndoGroups::const_iterator& end) const
{
    begin = m_appState.base();
    end = m_groups.end();
}

HistoryManagerTransaction::HistoryManagerTransaction()
{
    if(auto hm = HistoryManager::getInstance()) {
        hm->StartTransaction();
    }
}
HistoryManagerTransaction::~HistoryManagerTransaction()
{
    if(auto hm = HistoryManager::getInstance()) {
        hm->EndTransaction();
    }
}

HistoryManagerPause::HistoryManagerPause()
{
    if(auto hm = HistoryManager::getInstance()) {
        hm->PushPause();
    }
}
HistoryManagerPause::~HistoryManagerPause()
{
    if(auto hm = HistoryManager::getInstance()) {
        hm->PopPause();
    }
}
