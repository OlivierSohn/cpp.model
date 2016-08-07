
#include "os.log.h"

#include "history.manager.h"

#include "referentiable.cmd.set.h"

#include "referentiable.manager.h"

#include "referentiable.cmd.list.h"

namespace imajuscule {

REF_CMD_LIST
RefAttrListCmd<T,U,fAdd,fRemove>::CommandResult::CommandResult(bool bSuccess) :
Command::CommandResult(bSuccess)
{}


REF_CMD_LIST
RefAttrListCmd<T,U,fAdd,fRemove>::data::data(const U * attr, Type t) :
Command::data()
, m_manager(NULL)
, m_t(t)
{
    if (attr)
    {
        m_attrGUID = attr->guid();
        m_manager = attr->getManager();
    }
}


REF_CMD_LIST
bool RefAttrListCmd<T,U,fAdd,fRemove>::data::operator!=(const Command::data&other) const
{
    auto pOther = dynamic_cast<const RefAttrListCmd<T,U,fAdd,fRemove>::data * >(&other);
    if_A(pOther)
    {
        if(m_t != pOther->m_t)
            return true;
        if (m_attrGUID != pOther->m_attrGUID)
            return true;
        if (m_manager != pOther->m_manager)
            return true;

        return false;
    }
    return true;
}

REF_CMD_LIST
U * RefAttrListCmd<T,U,fAdd,fRemove>::data::Attr() const
{
    return static_cast<U *>(m_manager->findByGuid(m_attrGUID));
}
REF_CMD_LIST
Command::data * RefAttrListCmd<T,U,fAdd,fRemove>::data::instantiate(const U * attr, Type t){
    return new data(attr, t);
}
REF_CMD_LIST
std::string RefAttrListCmd<T,U,fAdd,fRemove>::data::getDesc() const
{
    std::string desc;
    
    switch(m_t)
    {
        case Type::TYPE_ADD:
            desc.append("add ");
            break;
            
        case Type::TYPE_REMOVE:
            desc.append("remove ");
            break;
            
        default:
            A(0);
            break;
    }

    if (const Referentiable * ref = Attr())
        desc.append(ref->sessionName());
    else
        desc.append("missing");
    
    
    return desc;
}

REF_CMD_LIST
RefAttrListCmd<T,U,fAdd,fRemove>::RefAttrListCmd(T & obj, const U * iAttr, Type t) :
Command(RefAttrListCmd<T,U,fAdd,fRemove>::data::instantiate(iAttr, Other(t)), RefAttrListCmd<T,U,fAdd,fRemove>::data::instantiate(iAttr, t), dynamic_cast<Referentiable*>(&obj))
, m_type(t)
{
}

REF_CMD_LIST
RefAttrListCmd<T,U,fAdd,fRemove>::~RefAttrListCmd()
{
}

REF_CMD_LIST
void RefAttrListCmd<T,U,fAdd,fRemove>::getSentenceDescription(std::string & desc) const
{
    desc.append(std::string("manage list : "));
}

REF_CMD_LIST
bool RefAttrListCmd<T,U,fAdd,fRemove>::ManageAttr(T & obj, U * Attr, Type t)
{
    bool bSuccess = true;
    
    if (HistoryManager::getInstance()->isActive())
    {
        if (!ExecuteFromInnerCommand(obj, Attr, t, bSuccess))
            bSuccess = Execute(obj, Attr, t);
    }
    else
    {
        switch(t)
        {
            case Type::TYPE_ADD:
                std::bind(fAdd, &obj, Attr)();
                break;
                
            case Type::TYPE_REMOVE:
                std::bind(fRemove, &obj, Attr)();
                break;
                
            default:
                bSuccess = false;
                A(0);
                break;
        }
    }
    
    return bSuccess;
}

REF_CMD_LIST
auto RefAttrListCmd<T,U,fAdd,fRemove>::Other(Type t) -> Type
{
    switch(t)
    {
        case Type::TYPE_ADD:
            return Type::TYPE_REMOVE;
            break;
            
        case Type::TYPE_REMOVE:
            return Type::TYPE_ADD;
            break;
            
        default:
            A(0);
            return Type::TYPE_ADD;
            break;
    }
}
REF_CMD_LIST
bool RefAttrListCmd<T,U,fAdd,fRemove>::ExecuteFromInnerCommand(T & obj, U * newAttr, Type t, bool & bSuccess)
{
    bSuccess = false;

    Command::data * before = data::instantiate(newAttr, Other(t));
    Command::data * after = data::instantiate(newAttr, t);

    CommandResult r;
    resFunc f(RESULT_BY_REF(r));

    bool bDone = Command::ExecuteFromInnerCommand<RefAttrListCmd>(
        *before,
        *after,
        dynamic_cast<Referentiable*>(&obj),
        &f);

    if (before)
        delete before;
    if (after)
        delete after;

    if (bDone)
    {
        bSuccess = r.Success();
    }

    return bDone;
}
REF_CMD_LIST
bool RefAttrListCmd<T,U,fAdd,fRemove>::doExecute(const Command::data & Data)
{
    bool bSuccess = false;

    const RefAttrListCmd<T,U,fAdd,fRemove>::data * pData = dynamic_cast<const RefAttrListCmd<T,U,fAdd,fRemove>::data*>(&Data);
    if_A(pData)
    {
        T * obj = dynamic_cast<T*>(getObject());
        if_A(obj)
        {
            switch(pData->m_t)
            {
                case Type::TYPE_ADD:
                    std::bind(fAdd, obj, pData->Attr())();
                    bSuccess = true;
                    break;
                    
                case Type::TYPE_REMOVE:
                    std::bind(fRemove, obj, pData->Attr())();
                    bSuccess = true;
                    break;
                    
                default:
                    A(0);
                    bSuccess = false;
                    break;
            }
        }
    }

    CommandResult r(bSuccess);
    observable().Notify(Event::RESULT, &r);

    return bSuccess;
}

REF_CMD_LIST
bool RefAttrListCmd<T,U,fAdd,fRemove>::Execute(T & obj, const U * iAttr, Type t)
{
    auto * c = new RefAttrListCmd(obj, iAttr, t);
    CommandResult r;
    auto reg = CommandResult::ListenToResult(*c, r);

    if (c->Command::Execute())
        c->observable().Remove(reg);

    return r.Success();
}
} //namespace imajuscule
