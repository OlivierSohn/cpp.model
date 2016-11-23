#include <memory>

#include "os.log.h"

#include "history.manager.h"

#include "referentiable.cmd.set.h"

#include "referentiable.manager.h"

#include "referentiable.cmd.list.h"

namespace imajuscule {
    
    REF_CMD_LIST
    RefAttrListCmd<T,U,fAdd,fRemove>::CommandResult::CommandResult(bool bSuccess, bool found) :
    found(found),
    Command::CommandResult(bSuccess)
    {}
    
    
    REF_CMD_LIST
    RefAttrListCmd<T,U,fAdd,fRemove>::data::data(const U * attr, Type t) :
    Command::data()
    , m_manager(nullptr)
    , m_t(t)
    {
        if (attr) {
            m_attrGUID = attr->guid();
            m_manager = attr->getManager();
        }
    }
    
    
    REF_CMD_LIST
    bool RefAttrListCmd<T,U,fAdd,fRemove>::data::operator!=(const Command::data&other) const
    {
        auto pOther = dynamic_cast<const RefAttrListCmd<T,U,fAdd,fRemove>::data * >(&other);
        A(pOther);
        if(m_t != pOther->m_t) {
            return true;
        }
        if (m_attrGUID != pOther->m_attrGUID) {
            return true;
        }
        if (m_manager != pOther->m_manager) {
            return true;
        }
        
        return false;
    }
    
    REF_CMD_LIST
    U * RefAttrListCmd<T,U,fAdd,fRemove>::data::Attr() const
    {
        if(!m_manager) {
            return nullptr;
        }
        return static_cast<U *>(m_manager->findByGuid(m_attrGUID));
    }
    
    REF_CMD_LIST
    Command::data * RefAttrListCmd<T,U,fAdd,fRemove>::data::instantiate(const U * attr, Type t){
        return new data(attr, t);
    }
    
    REF_CMD_LIST
    std::string RefAttrListCmd<T,U,fAdd,fRemove>::data::getDesc() const {
        std::string desc;
        
        switch(m_t) {
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
        
        if (const auto * ref = Attr()) {
            desc.append(ref->sessionName());
        }
        else {
            desc.append("missing");
        }
        
        return desc;
    }
    
    REF_CMD_LIST
    RefAttrListCmd<T,U,fAdd,fRemove>::RefAttrListCmd(T & obj, const U * iAttr, Type t) :
    Command(RefAttrListCmd<T,U,fAdd,fRemove>::data::instantiate(iAttr, Other(t)), RefAttrListCmd<T,U,fAdd,fRemove>::data::instantiate(iAttr, t), dynamic_cast<Referentiable*>(&obj))
    , m_type(t)
    {
    }
    
    REF_CMD_LIST
    void RefAttrListCmd<T,U,fAdd,fRemove>::getSentenceDescription(std::string & desc) const
    {
        desc.append(std::string("manage list : "));
    }
    
    REF_CMD_LIST
    bool RefAttrListCmd<T,U,fAdd,fRemove>::ManageAttr(T & obj, U * Attr, Type t, bool & found)
    {
        bool bSuccess = true;
        
        auto hm = HistoryManager::getInstance();
        if (hm && hm->isActive()) {
            if (!ExecuteFromInnerCommand(obj, Attr, t, bSuccess, found)) {
                bSuccess = Execute(obj, Attr, t, found);
            }
        }
        else {
            switch(t) {
                case Type::TYPE_ADD:
                    std::bind(fAdd, &obj, Attr)();
                    found = true;
                    break;
                    
                case Type::TYPE_REMOVE:
                    found = std::bind(fRemove, &obj, Attr)();
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
    auto RefAttrListCmd<T,U,fAdd,fRemove>::Other(Type t) -> Type {
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
    bool RefAttrListCmd<T,U,fAdd,fRemove>::ExecuteFromInnerCommand(T & obj, U * newAttr, Type t, bool & bSuccess, bool & found)
    {
        bSuccess = false;
        
        auto before = data::instantiate(newAttr, Other(t));
        auto after = data::instantiate(newAttr, t);
        
        CommandResult r;
        resFunc f(RESULT_BY_REF(r));
        
        bool bDone = Command::ExecuteFromInnerCommand<RefAttrListCmd>(
                                                                      *before,
                                                                      *after,
                                                                      dynamic_cast<Referentiable*>(&obj),
                                                                      &f);
        
        if (bDone)
        {
            bSuccess = r.Success();
            found = r.getFound();
        }
        
        return bDone;
    }
    
    REF_CMD_LIST
    bool RefAttrListCmd<T,U,fAdd,fRemove>::doExecute(const Command::data & Data)
    {
        bool bSuccess = false;
        bool found = true;
        
        auto * pData = dynamic_cast<const RefAttrListCmd<T,U,fAdd,fRemove>::data*>(&Data);
        A(pData);
        auto * obj = dynamic_cast<T*>(getObject());
        A(obj);
        switch(pData->m_t)
        {
            case Type::TYPE_ADD:
                std::bind(fAdd, obj, pData->Attr())();
                bSuccess = true;
                break;
                
            case Type::TYPE_REMOVE:
                found = std::bind(fRemove, obj, pData->Attr())();
                bSuccess = true;
                break;
                
            default:
                A(0);
                bSuccess = false;
                break;
        }
        
        CommandResult r(bSuccess, found);
        observable().Notify(Event::RESULT, &r);
        
        return bSuccess;
    }
    
    REF_CMD_LIST
    bool RefAttrListCmd<T,U,fAdd,fRemove>::Execute(T & obj, const U * iAttr, Type t, bool & found)
    {
        auto * c = new RefAttrListCmd(obj, iAttr, t);
        CommandResult r;
        auto reg = CommandResult::ListenToResult(*c, r);
        
        if (c->Command::Execute()) {
            c->observable().Remove(reg);
        }
        
        found = r.getFound();
        return r.Success();
    }
} //namespace imajuscule
