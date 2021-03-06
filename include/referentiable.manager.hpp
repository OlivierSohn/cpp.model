
using namespace imajuscule;

ReferentiableManagerBase::ReferentiableManagerBase():
Visitable()
, m_observable(Observable<Event, Referentiable*>::instantiate())
, session_name_last_suffix(0)
{
}

ReferentiableManagerBase::~ReferentiableManagerBase()
{
    observable().Notify(Event::MANAGER_DELETE, {});
    observable().deinstantiate();

    // doc F3F7C744-0B78-4750-A0A1-7A9BAD872188
    for(auto ref : refs) {
        LG(ERR, "ref %s was not deinstantiated", ref->sessionName().c_str());
    }
}

bool ReferentiableManagerBase::RegisterWithSessionName(Referentiable * r, const std::string & sessionName)
{
    Assert(r);
    r->setSessionName(sessionName);
    {
        auto const & guid = r->guid();
        auto it = m_guidsToRftbls.find(guid);
        if (it != m_guidsToRftbls.end()) {
            Assert(!"guid already present");
            return false;
        }

        m_guidsToRftbls.insert(it, guidsToRftbls::value_type(guid, r));
    }
    
    auto it = m_snsToRftbls.find(sessionName);
    if (likely(it == m_snsToRftbls.end())) {
        m_snsToRftbls.insert(it, guidsToRftbls::value_type(sessionName, r));
    }
    else {
        Assert(!"an element was not found in guid map but found in session names map!");
    }
    
    refs.push_back(r);

    return true;
}

void ReferentiableManagerBase::RemoveRefInternal(Referentiable*r)
{
    Assert(r);
    // copies are intentional (the string need to outlive the referentiable)
    std::string guid = r->guid();
    std::string sessionName = r->sessionName();
    
    //LG(INFO, "delete %s %s", guid.c_str(), sessionName.c_str());
    
    // during destruction, the object must be accessible via its manager
    // because for example when destructing a joint, we need to launch a command to change the parent to nullptr and this command uses the manager to find the object
    // that's why delete is done before removing guid and session name from maps
    r->observableReferentiable()->Notify(Referentiable::Event::WILL_BE_DELETED, r);
    // to make sure that the observable is not used during the destructor
    r->deleteObservableReferentiable();
    delete r;
    
    auto count = m_guidsToRftbls.erase(guid);
    Assert(count == 1);
    
    count = m_snsToRftbls.erase(sessionName);
    Assert(count == 1);
    
    auto it = refs.begin();
    for(; it != refs.end(); ++it) {
        if(*it == r) {
            auto ptr = *it;
            refs.erase(it);
            observable().Notify(Event::RFTBL_REMOVE, ptr); // must be placed after actual delete (use case : delete of joint makes the parent nullptr so the joint ui manager draws a joint at root which must be removed)
            break;
        }
    }
    
    Assert(refs.size() == m_guidsToRftbls.size());
    Assert(refs.size() == m_snsToRftbls.size());
    
}

struct pred
{
    bool operator()(Referentiable const * a, Referentiable const * b) const
    {
        Assert(a && b);
        std::string date1 = a->creationDate();
        std::string date2 = b->creationDate();
        FormatDateForComparison(date1);
        FormatDateForComparison(date2);

        return date1 < date2;
    }
};

referentiables ReferentiableManagerBase::ListReferentiablesByCreationDate() const
{
    referentiables vItems = refs;

    std::sort(vItems.begin(), vItems.end(), pred());
    
    return vItems;
}

Referentiable * ReferentiableManagerBase::findByGuid(const std::string & guid)
{
    //LG(INFO, "ReferentiableManagerBase::findByGuid(%s)", guid.c_str());
    guidsToRftbls::iterator it = m_guidsToRftbls.find(guidsToRftbls::key_type(guid));
    if (it != m_guidsToRftbls.end()) {
        return it->second;
    }
    return {};
}
// session name is unique per-session
Referentiable * ReferentiableManagerBase::findBySessionName(const std::string & sessionName)
{
    snsToRftbls::iterator it = m_snsToRftbls.find(snsToRftbls::key_type(sessionName));
    if (it != m_snsToRftbls.end()) {
        return it->second;
    }
    return {};
}

bool ReferentiableManagerBase::ComputeSessionName(Referentiable * r, bool bFinalize)
{
    Assert(r);
    std::string sessionName = r->hintName();
        
    if(findBySessionName(sessionName))
    {
        sessionName += "_";
        
        auto f = [this](const std::string & pre, int i) -> bool {
            return findBySessionName(pre + std::to_string(i)) ? true: false;
        };
        
        while (f(sessionName,session_name_last_suffix)) {
            session_name_last_suffix++;
        }
        
        sessionName += std::to_string(session_name_last_suffix);
        session_name_last_suffix++;
    }
    if(!RegisterWithSessionName(r, sessionName)) {
        return false;
    }
    if(bFinalize) {
        r->Init();
        r->onLoaded();
        
        observable().Notify(ReferentiableManagerBase::Event::RFTBL_ADD, r);
    }
    return true;
}


void ReferentiableManagerBase::RemoveRef(Referentiable*r)
{
    Assert(r);

    auto * h = HistoryManager::getInstance();
    
    if (h->isActive())
    {
        if (!ReferentiableDeleteCmdBase::ExecuteFromInnerCommand(*r)) {
            ReferentiableDeleteCmdBase::Execute(*r);
        }
    }
    else {
        RemoveRefInternal(r);
    }    
}

ref_unique_ptr<Referentiable> ReferentiableManagerBase::newReferentiable(bool bFinalize)
{
    return newReferentiable(defaultNameHint(), bFinalize);
}

ref_unique_ptr<Referentiable> ReferentiableManagerBase::newReferentiable(const std::string & nameHint, bool bFinalize)
{
    return newReferentiable(nameHint, std::vector < std::string>(), bFinalize);
}
ref_unique_ptr<Referentiable> ReferentiableManagerBase::newReferentiable(const std::string & nameHint, const std::vector<std::string> & guids, bool bFinalize)
{
    HistoryManager * h = HistoryManager::getInstance();

    if (!h->isActive()) {
        return newReferentiableInternal(nameHint, guids, bFinalize);
    }
    Assert(bFinalize);
    ref_unique_ptr<Referentiable> r;
    if (ReferentiableNewCmdBase::ExecuteFromInnerCommand(*this, nameHint, guids, r)) {
        return r;
    }
    return ReferentiableNewCmdBase::Execute(*this, nameHint, guids);
}

template <class T>
ReferentiableManager<T> * ReferentiableManager<T>::g_pRefManager = nullptr;

template <class T>
ReferentiableManager<T> * ReferentiableManager<T>::getInstance()
{
    return Globals::ptr<ReferentiableManager<T>>(g_pRefManager);
}


template <class T>
ref_unique_ptr<Referentiable> ReferentiableManager<T>::newReferentiableInternal(const std::string & nameHint, const std::vector<std::string> & guids, bool bFinalize)
{
    /*LG(INFO, "ReferentiableManager<T>::newReferentiable(%s, %d guids) begin",
        (nameHint.c_str() ? nameHint.c_str() : "nullptr"),
        guids.size());*/

    std::string guid;

    if (!guids.empty()) {
        guid.assign(guids[0]);
    }
    else {
        guid = generateGuid();
    }

    auto ref = ref_unique_ptr<T>(new T(this, std::move(guid), nameHint));
    if (unlikely(!ComputeSessionName(ref.get(), bFinalize))) {
        LG(ERR, "ReferentiableManager<T>::newReferentiable : ComputeSessionName failed");
        return {};
    }

    //LG((ref ? INFO : ERR), "ReferentiableManager<T>::newReferentiable(...) returns 0x%x", ref);
    return {ref.release()};
}

template <class T>
ref_unique_ptr<T> ReferentiableManager<T>::New()
{
    auto * rm = ReferentiableManager<T>::getInstance();
    if(!rm) {
        return {};
    }
    return {safe_cast<T*>(rm->newReferentiable(true).release())};
}

bool ReferentiableCmdBase::data::operator!=(const Undoable::data& other) const
{
    auto pOther = dynamic_cast<const ReferentiableCmdBase::data * >(&other);
    Assert(pOther);
    if (m_manager != pOther->m_manager) {
        return true;
    }
    if (m_action != pOther->m_action) {
        return true;
    }
    if (m_hintName != pOther->m_hintName) {
        return true;
    }
    
    return false;
}

std::string ReferentiableCmdBase::data::getDesc() const {
    return {};
}

ReferentiableCmdBase::data::data(Action a, std::string hintName, ReferentiableManagerBase * rm):
Undoable::data()
, m_action(a)
, m_hintName(hintName)
, m_manager(rm)
{}

auto ReferentiableCmdBase::data::instantiate(Action a, std::string hintName, ReferentiableManagerBase * rm) -> data* {
    return new data(a, hintName, rm);
}

auto ReferentiableCmdBase::other(Action a)->Action
{
    switch (a)
    {
    case ACTION_DELETE:
        return ACTION_NEW;
        break;
    case ACTION_NEW:
        return ACTION_DELETE;
        break;
    default:
    case ACTION_UNKNOWN:
        Assert(0);
        return ACTION_UNKNOWN;
        break;
    }
}

ReferentiableCmdBase::ReferentiableCmdBase(ReferentiableManagerBase * manager, const std::string & nameHint, Action action) :
Command(new data(other(action), nameHint, manager), new data(action, nameHint, manager))
, m_manager(manager)
, m_hintName(nameHint)
{}


bool ReferentiableCmdBase::doExecute(const Undoable::data & data)
{
    bool bDone = false;

    auto pData = dynamic_cast<const ReferentiableCmdBase::data*>(&data);
    Assert(pData);
    switch (pData->m_action)
    {
        case ACTION_DELETE:
            doDeinstantiate();
            bDone = true;
            break;
        case ACTION_NEW:
            doInstantiate();
            bDone = true;
            break;
        case ACTION_UNKNOWN:
            Assert(!"unknown action");
            break;
    }
    return bDone;
}

void ReferentiableCmdBase::doInstantiate()
{
    std::vector<std::string> guids;
    bool bGUID = false;
    if (!m_GUID.empty()) {
        bGUID = true;
        guids.push_back(m_GUID);
    }
    // F3F7C744-0B78-4750-A0A1-7A9BAD872188
    auto r = manager()->newReferentiableInternal(m_hintName, guids).release();
    Assert(r);
    if (!bGUID) {
        m_GUID = r->guid();
    }
    else {
        Assert(m_GUID == r->guid());
    }
    
    Assert(m_hintName == r->hintName());

    CommandResult res(true, r);
    observable().Notify(Event::RESULT, &res);
}

void ReferentiableCmdBase::doDeinstantiate()
{
    auto r = manager()->findByGuid(m_GUID);
    Assert(r);
    manager()->RemoveRefInternal(r);
}

ReferentiableCmdBase::CommandResult::CommandResult(bool bSuccess, Referentiable*ref):
Undoable::CommandResult(bSuccess)
, m_addr(ref)
{}


ReferentiableNewCmdBase::ReferentiableNewCmdBase(ReferentiableManagerBase & manager, const std::string & nameHint, const std::vector<std::string> guids) :
ReferentiableCmdBase(&manager, nameHint, ACTION_NEW)
, m_guids(guids)
{
    if(!guids.empty()) {
        m_GUID = guids.front();
    }
}

void ReferentiableNewCmdBase::Instantiate()
{
    if (getState() == NOT_EXECUTED) {
        Command::Execute();
    }
    else {
        Redo();
    }
}
void ReferentiableNewCmdBase::Deinstantiate() {
    Undo();
}

void ReferentiableNewCmdBase::getSentenceDescription(std::string & desc) const {
    desc.append("new Ref. \"");
    desc.append(m_hintName);
    desc.append("\"");
}

bool ReferentiableNewCmdBase::ExecuteFromInnerCommand(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids, ref_unique_ptr<Referentiable>& oRefAddr)
{
    oRefAddr = nullptr;

    std::unique_ptr<Undoable::data> before(data::instantiate(ACTION_DELETE, nameHint, &rm));
    std::unique_ptr<Undoable::data> after(data::instantiate(ACTION_NEW, nameHint, &rm));

    CommandResult r;
    resFunc f(RESULT_BY_REF(r));

    if(!Undoable::ExecuteFromInnerCommand<ReferentiableCmdBase>(*before, *after, {}, &f)) {
        return false;
    }
    
    Assert(r.Success());
    oRefAddr = r.addr();
    return true;
}

ref_unique_ptr<Referentiable> ReferentiableNewCmdBase::Execute(ReferentiableManagerBase & rm, const std::string & nameHint, const std::vector<std::string> guids)
{
    auto c = new ReferentiableNewCmdBase(rm, nameHint, guids);
    CommandResult r;
    auto reg = CommandResult::ListenToResult(*c, r);

    if (c->Command::Execute()) {
        c->observable().Remove(reg);
    }

    return (r.Success() ?
            r.addr()
            :
            nullptr);
}

std::string const & ReferentiableCmdBase::guid() const {
    return m_GUID;
}

std::string const & ReferentiableCmdBase::hintName() const {
    return m_hintName;
}

ReferentiableManagerBase * ReferentiableCmdBase::manager() const {
    Assert(m_manager);
    return m_manager;
}

ReferentiableDeleteCmdBase::ReferentiableDeleteCmdBase(Referentiable & r) :
ReferentiableCmdBase(r.getManager(), r.hintName(), ACTION_DELETE)
{
    m_GUID = r.guid();
}

void ReferentiableDeleteCmdBase::getSentenceDescription(std::string & desc) const
{
    desc.append("delete Ref. \"");
    desc.append(m_hintName);
    desc.append("\"");
}

void ReferentiableDeleteCmdBase::Instantiate(){
    Undo();
}

void ReferentiableDeleteCmdBase::Deinstantiate() {
    if (getState() == NOT_EXECUTED) {
        Command::Execute();
    }
    else {
        Redo();
    }
}

bool ReferentiableDeleteCmdBase::ExecuteFromInnerCommand(Referentiable & r)
{
    std::string nameHint = r.hintName();
    auto * rm = r.getManager();
    
    std::unique_ptr<Undoable::data> before(data::instantiate(ACTION_NEW, nameHint, rm));
    std::unique_ptr<Undoable::data> after(data::instantiate(ACTION_DELETE, nameHint, rm));

    bool bDone = Undoable::ExecuteFromInnerCommand<ReferentiableCmdBase>(
        *before,
        *after);

    return bDone;
}
void ReferentiableDeleteCmdBase::Execute(Referentiable & r)
{
    (new ReferentiableDeleteCmdBase(r))->Command::Execute();
}

