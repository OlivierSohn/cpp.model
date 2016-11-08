#include "os.log.h"

#include "globals.h"

#include "referentiables.h"
#include "referentiable.manager.h"

#include "referentiable.root.h"

const int KEY_TARGETS = -110;

using namespace imajuscule;

ReferentiableRoot * ReferentiableRoot::g_instance = nullptr;

IMPL_PERSIST2(ReferentiableRoot, Referentiable,
            ,
            ,
            );

ReferentiableRoot * ReferentiableRoot::getInstance()
{
    return Globals::ref<ReferentiableRoot>(g_instance);
}

ReferentiableRoot::ReferentiableRoot(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName) :
Referentiable(manager, guid, hintName)
{}

void ReferentiableRoot::initialize() {
    auto refroot_manager = getManager();
    for(auto * m : Referentiables::getManagers())
    {
        if(!m) {
            continue;
        }

        if(m == refroot_manager) {
            continue;
        }
        for (auto vi : m->traverse() ) {
            addRef(vi);
        }
        
        m->observable().Register(ReferentiableManagerBase::Event::RFTBL_ADD,
                                  [this](Referentiable*r){
                                      if(!dynamic_cast<ReferentiableRoot*>(r))
                                          this->addRef(r);
                                  });
    }
}

void ReferentiableRoot::addRef(Referentiable* ref)
{
    //LG(INFO,"+ %x, %s", ref, ref->sessionName().c_str());
    A(ref);
    auto it = m_refs.find(refs::key_type( ref ));
    if_A(it == m_refs.end())
    {
        m_refs.insert(ref);
        addSpec(ref);
        if(auto refobs = ref->observableReferentiable()) {
            refobs->Register(Referentiable::Event::WILL_BE_DELETED,
                             [this](Referentiable*r){
                                 if(r!= this)
                                     this->removeRef(r);
                             });
        }
    }
}
void ReferentiableRoot::removeRef(Referentiable* ref)
{
    //LG(INFO,"- %x", ref);
    A(ref);
    auto it = m_refs.find(ref);
    A(it != m_refs.end());

    m_refs.erase(it);
    removeSpec(ref);
}
