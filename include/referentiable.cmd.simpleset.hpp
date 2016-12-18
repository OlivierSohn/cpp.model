
using namespace imajuscule;


REF_CMD_SIMPLESET
RefSimpleChangeAttrCmd<T,U,fSet,fGet>::CommandResult::CommandResult(bool bSuccess) :
Command::CommandResult(bSuccess)
{}

REF_CMD_SIMPLESET
RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data::data(const U * attr) :
Command::data()
, m_hasAttr(false)
, m_manager(nullptr)
{
    if (attr) {
        m_hasAttr = true;
        m_attrGUID = attr->guid();
        m_manager = attr->getManager();
    }
}

REF_CMD_SIMPLESET
RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data::data(T&obj) :
Command::data()
, m_hasAttr(false)
, m_manager(nullptr)
{
    if (auto attr = std::bind(fGet, &obj)())  {
        m_hasAttr = true;
        m_attrGUID = attr->guid();
        m_manager = attr->getManager();
    }
}

REF_CMD_SIMPLESET
bool RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data::operator!=(const Command::data&other) const
{
    auto pOther = dynamic_cast<const RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data * >(&other);
    A(pOther);
    if (m_hasAttr != pOther->m_hasAttr) {
        return true;
    }
    if (m_hasAttr && (m_attrGUID != pOther->m_attrGUID)) {
        return true;
    }
    if (m_manager != pOther->m_manager) {
        return true;
    }
    return false;
}

REF_CMD_SIMPLESET
U * RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data::Attr() const {
    if (!m_hasAttr) {
        return {};
    }
    return static_cast<U *>(m_manager->findByGuid(m_attrGUID));
}

REF_CMD_SIMPLESET
Command::data * RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data::instantiate(const U * attr){
    return new data(attr);
}
REF_CMD_SIMPLESET
Command::data * RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data::instantiate(T& obj){
    return new data(obj);
}
REF_CMD_SIMPLESET
std::string RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data::getDesc() const
{
    if (!m_hasAttr) {
        return "none";
    }
    
    if (auto ref = Attr()) {
        return ref->sessionName();
    }
    return "missing";
}

REF_CMD_SIMPLESET
RefSimpleChangeAttrCmd<T,U,fSet,fGet>::RefSimpleChangeAttrCmd(T & obj, const U * iAttr) :
Command(RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data::instantiate(obj), RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data::instantiate(iAttr), dynamic_cast<Referentiable*>(&obj))
{
}

REF_CMD_SIMPLESET
void RefSimpleChangeAttrCmd<T,U,fSet,fGet>::getSentenceDescription(std::string & desc) const {
    desc.append(std::string(" attr : "));
}

REF_CMD_SIMPLESET
void RefSimpleChangeAttrCmd<T, U, fSet, fGet>::ChangeAttr(T & obj, U * newAttr)
{
    bool bSuccess = true;
    
    if (newAttr != std::bind(fGet,&obj)())
    {
        auto hm = HistoryManager::getInstance();
        if (hm && hm->isActive())
        {
            if (!ExecuteFromInnerCommand(obj, newAttr, bSuccess)) {
                bSuccess = Execute(obj, newAttr);
            }
        }
        else {
            std::bind(fSet, &obj, newAttr)();
        }
    }
}


REF_CMD_SIMPLESET
bool RefSimpleChangeAttrCmd<T,U,fSet,fGet>::ExecuteFromInnerCommand(T & obj, U * newAttr, bool & bSuccess)
{
    bSuccess = false;
    
    std::unique_ptr<Command::data> before(data::instantiate(obj));
    std::unique_ptr<Command::data> after(data::instantiate(newAttr));
    
    CommandResult r;
    resFunc f(RESULT_BY_REF(r));
    
    bool bDone = Command::ExecuteFromInnerCommand<RefSimpleChangeAttrCmd>(
                                                                          *before,
                                                                          *after,
                                                                          dynamic_cast<Referentiable*>(&obj),
                                                                          &f);
    
    if (bDone) {
        bSuccess = r.Success();
    }
    
    return bDone;
}
REF_CMD_SIMPLESET
bool RefSimpleChangeAttrCmd<T,U,fSet,fGet>::doExecute(const Command::data & Data)
{
    bool bSuccess = false;
    
    auto pData = dynamic_cast<const RefSimpleChangeAttrCmd<T,U,fSet,fGet>::data*>(&Data);
    A(pData);
    T * obj = dynamic_cast<T*>(getObject());
    A(obj);
    bSuccess = true;
    std::bind(fSet, obj, pData->Attr())();
    
    CommandResult r(bSuccess);
    observable().Notify(Event::RESULT, &r);
    
    return bSuccess;
}

REF_CMD_SIMPLESET
bool RefSimpleChangeAttrCmd<T,U,fSet,fGet>::Execute(T & obj, const U * iAttr)
{
    auto * c = new RefSimpleChangeAttrCmd(obj, iAttr);
    CommandResult r;
    auto reg = CommandResult::ListenToResult(*c, r);
    
    if (c->Command::Execute()) {
        c->observable().Remove(reg);
    }
    
    return r.Success();
}
