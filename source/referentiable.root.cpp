#include "os.log.h"

#include "referentiables.h"
#include "referentiable.manager.h"

#include "referentiable.root.h"

#define KEY_TARGETS           -110

using namespace imajuscule;

ReferentiableRoot * ReferentiableRoot::g_instance = NULL;

IMPL_PERSIST2(ReferentiableRoot, Referentiable,
            ,
            ,
            );

ReferentiableRoot * ReferentiableRoot::getInstance()
{
    if(!g_instance)
        g_instance = NEWREF(ReferentiableRoot);
    return g_instance;
}

ReferentiableRoot::ReferentiableRoot(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName) :
Referentiable(manager, guid, hintName)
{
    managers::iterator it, end;
    Referentiables::traverseManagers(it, end);
    for(;it!=end;++it)
    {
        ReferentiableManagerBase * rm = *it;
        
        referentiables v;
        rm->ListReferentiablesByCreationDate(v);
        for (auto & vi : v)
            addRef(vi);
        
        rm->observable().Register(ReferentiableManagerBase::Event::RFTBL_ADD,
                                  [this](Referentiable*r){
                                      if(!dynamic_cast<ReferentiableRoot*>(r))
                                          this->addRef(r);
                                  });
    }
}
ReferentiableRoot::~ReferentiableRoot()
{
}

void ReferentiableRoot::addRef(Referentiable* ref)
{
    //LG(INFO,"+ %x, %s", ref, ref->sessionName().c_str());
    A(ref);
    auto it = m_refs.find(refs::key_type( ref ));
    if_A(it == m_refs.end())
    {
        m_refs.insert(refs::value_type(ref,CLINK(Referentiable,ref)));
        addSpec(ref);
        ref->observableReferentiable().Register(Referentiable::Event::WILL_BE_DELETED,
                                  [this](Referentiable*r){
                                      if(r!= this)
                                          this->removeRef(r);
                                  });
    }
}
void ReferentiableRoot::removeRef(Referentiable* ref)
{
    //LG(INFO,"- %x", ref);
    A(ref);
    auto it = m_refs.find(refs::key_type( ref ));
    if_A(it != m_refs.end())
    {
        m_refs.erase(it);
        removeSpec(ref);
    }
}
