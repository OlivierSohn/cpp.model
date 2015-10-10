#include "referentiable.cmd.set.h"
#include "history.manager.h"
#include "os.log.h"

using namespace imajuscule;


REF_CMD_SET
RefChangeAttrCmd<T,U,fSet,fGet>::CommandResult::CommandResult(bool bSuccess, bool bAttrChanged) :
Command::CommandResult(bSuccess)
, m_bAttrChanged(bAttrChanged)
{}

REF_CMD_SET
bool RefChangeAttrCmd<T,U,fSet,fGet>::CommandResult::getAttrChanged() const
{
    A(initialized());
    return m_bAttrChanged;
}

REF_CMD_SET
RefChangeAttrCmd<T,U,fSet,fGet>::data::data(const U * attr) :
Command::data()
, m_hasAttr(false)
, m_manager(NULL)
{
    if (attr)
    {
        m_hasAttr = true;
        m_attrGUID = attr->guid();
        m_manager = attr->getManager();
    }
}

REF_CMD_SET
RefChangeAttrCmd<T,U,fSet,fGet>::data::data(T&obj) :
Command::data()
, m_hasAttr(false)
, m_manager(NULL)
{
    if (U * attr = std::bind(fGet, &obj)())
    {
        m_hasAttr = true;
        m_attrGUID = attr->guid();
        m_manager = attr->getManager();
    }
}

REF_CMD_SET
bool RefChangeAttrCmd<T,U,fSet,fGet>::data::operator!=(const Command::data&other) const
{
    auto pOther = dynamic_cast<const RefChangeAttrCmd<T,U,fSet,fGet>::data * >(&other);
    if_A(pOther)
    {
        if (m_hasAttr != pOther->m_hasAttr)
            return true;
        if (m_hasAttr && (m_attrGUID != pOther->m_attrGUID))
            return true;
        if (m_manager != pOther->m_manager)
            return true;

        return false;
    }
    return true;
}

REF_CMD_SET
U * RefChangeAttrCmd<T,U,fSet,fGet>::data::Attr() const
{
    U * j = NULL;

    if (m_hasAttr)
    {
        j = static_cast<U *>(m_manager->findByGuid(m_attrGUID));
    }
    return j;

}
REF_CMD_SET
Command::data * RefChangeAttrCmd<T,U,fSet,fGet>::data::instantiate(const U * attr){
    return new data(attr);
}
REF_CMD_SET
Command::data * RefChangeAttrCmd<T,U,fSet,fGet>::data::instantiate(T& obj){
    return new data(obj);
}
REF_CMD_SET
std::string RefChangeAttrCmd<T,U,fSet,fGet>::data::getDesc() const
{
    std::string desc;
    if (m_hasAttr)
    {
        if (Referentiable * ref = Attr())
            desc.append(ref->sessionName());
        else
            desc.append("missing");
    }
    else
        desc.append("none");

    return desc;
}

REF_CMD_SET
RefChangeAttrCmd<T,U,fSet,fGet>::RefChangeAttrCmd(T & obj, const U * iAttr) :
Command(RefChangeAttrCmd<T,U,fSet,fGet>::data::instantiate(obj), RefChangeAttrCmd<T,U,fSet,fGet>::data::instantiate(iAttr), dynamic_cast<Referentiable*>(&obj))
, m_preconditionning(NULL)
{
}

REF_CMD_SET
RefChangeAttrCmd<T,U,fSet,fGet>::~RefChangeAttrCmd()
{
    // commented out because preconditionnning command is an inner command so it will be deleted in Commmand::~Commmand()
    // delete m_preconditionning;
    m_preconditionning = NULL;
}

REF_CMD_SET
void RefChangeAttrCmd<T,U,fSet,fGet>::getSentenceDescription(std::string & desc) const
{
    if (m_preconditionning)
        desc.append(" (prec.)");
    desc.append(std::string(" attr : "));
}

REF_CMD_SET
bool RefChangeAttrCmd<T, U, fSet, fGet>::ChangeAttr(T & obj, U * newAttr, bool & bAttrChanged)
{
    bool bSuccess = true;
    bAttrChanged = false;

    if (newAttr != std::bind(fGet,&obj)())
    {
        if (HistoryManager::getInstance()->isActive())
        {
            if (!ExecuteFromInnerCommand(obj, newAttr, bSuccess, bAttrChanged))
                bSuccess = Execute(obj, newAttr, bAttrChanged);
        }
        else
        {
            bSuccess = std::bind(fSet, &obj, newAttr, std::ref(bAttrChanged))();
        }
    }
    return bSuccess;
}


REF_CMD_SET
bool RefChangeAttrCmd<T,U,fSet,fGet>::ExecuteFromInnerCommand(T & obj, U * newAttr, bool & bSuccess, bool & bAttrChanged)
{
    bSuccess = false;
    bAttrChanged = false;

    Command::data * before = data::instantiate(obj);
    Command::data * after = data::instantiate(newAttr);

    CommandResult r;
    resFunc f(RESULT_BY_REF(r));

    bool bDone = Command::ExecuteFromInnerCommand<RefChangeAttrCmd>(
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
        bAttrChanged = r.getAttrChanged();
    }

    return bDone;
}
REF_CMD_SET
bool RefChangeAttrCmd<T,U,fSet,fGet>::doExecute(const Command::data & Data)
{
    bool bSuccess = false;
    bool bChanged = false;

    const RefChangeAttrCmd<T,U,fSet,fGet>::data * pData = dynamic_cast<const RefChangeAttrCmd<T,U,fSet,fGet>::data*>(&Data);
    if_A(pData)
    {
        T * obj = dynamic_cast<T*>(getObject());
        if_A(obj)
        {
            bSuccess = std::bind(fSet, obj, pData->Attr(), std::ref(bChanged))();
        }
    }

    CommandResult r(bSuccess, bChanged);
    observable().Notify(Event::RESULT, &r);

    return bChanged; // because we don't want to log in history a command that would not do anything
}

REF_CMD_SET
bool RefChangeAttrCmd<T,U,fSet,fGet>::doExecute()
{
    bool bChanged = false;

    T * obj = dynamic_cast<T*>(getObject());

    if (U * newAttr = ((data*)After())->Attr())
    {
        if (T * Attr0 = dynamic_cast<T*>(newAttr))
        {
            while (T * Attr1 = dynamic_cast<T*>(std::bind(fGet, Attr0)()))
            {
                if (Attr1 == obj)
                {
                    // "obj" is an ancestor of newAttr so we add a preconditionning command that removes "obj" from newAttr's ancestors
                    m_preconditionning = new RefChangeAttrCmd<T, U, fSet, fGet>(*Attr0, std::bind(fGet, obj)());
                    break;
                }

                Attr0 = Attr1;
            }
        }
    }

    if (m_preconditionning)
    {
        bChanged = m_preconditionning->Execute();

        if (unlikely(!bChanged))
        {
            A(!"preconditionning command failed");
            // deleted by previous call
            m_preconditionning = NULL;
        }
    }

    if (Command::doExecute())
        bChanged = true;

    return bChanged;
}

REF_CMD_SET
bool RefChangeAttrCmd<T,U,fSet,fGet>::doUndo()
{
    bool bRelevant = Command::doUndo();

    if (m_preconditionning)
        if (m_preconditionning->Undo())
            bRelevant = true;

    return bRelevant;
}

REF_CMD_SET
bool RefChangeAttrCmd<T,U,fSet,fGet>::doRedo()
{
    bool bRelevant = false;

    if (m_preconditionning)
        bRelevant = m_preconditionning->Redo();

    if (Command::doRedo())
        bRelevant = true;

    return bRelevant;
}

REF_CMD_SET
bool RefChangeAttrCmd<T,U,fSet,fGet>::Execute(T & obj, const U * iAttr, bool & bAttrChanged)
{
    auto * c = new RefChangeAttrCmd(obj, iAttr);
    CommandResult r;
    auto reg = CommandResult::ListenToResult(*c, r);

    if (c->Command::Execute())
        c->observable().Remove(reg);

    bAttrChanged = r.getAttrChanged();
    return r.Success();
}