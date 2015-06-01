#include "referentiable.root.h"
#include "referentiable.manager.h"
#include "os.log.h"

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
        rm->observable().Register(ReferentiableManagerBase::Event::RFTBL_REMOVE,
                                  [this](Referentiable*r){
                                      if(r!= this)
                                          this->removeRef(r);
                                  });
    }
}
ReferentiableRoot::~ReferentiableRoot()
{
}

void ReferentiableRoot::addRef(Referentiable* ref)
{
    A(ref);
    m_refs.insert(refs::value_type(ref,CLINK(Referentiable,ref)));
}
void ReferentiableRoot::removeRef(Referentiable* ref)
{
    A(ref);
    auto it = m_refs.find(refs::key_type( ref ));
    if_A(it != m_refs.end())
    {
        m_refs.erase(it);
    }
}
