#include "os.log.h"

#include "referentiables.h"
#include "referentiable.manager.h"

#include "referentiable.root.h"

const int KEY_TARGETS = -110;

using namespace imajuscule;

ReferentiableRoot * ReferentiableRoot::g_instance = NULL;

IMPL_PERSIST2(ReferentiableRoot, Referentiable,
            ,
            ,
            );

ReferentiableRoot * ReferentiableRoot::getInstance()
{
    if(!g_instance)
        g_instance = MAKE_UNIQUE(ReferentiableRoot).release();
    return g_instance;
}

void ReferentiableRoot::teardown() {
    if(!g_instance) {
        return;
    }
    g_instance->deinstantiate();
    g_instance = 0;
}

void ReferentiableRoot::recycle_with_leak() {  // doc F3F7C744-0B78-4750-A0A1-7A9BAD872188
    if(!g_instance) {
        return;
    }
    g_instance = 0;
}

ReferentiableRoot::ReferentiableRoot(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName) :
Referentiable(manager, guid, hintName)
{
    auto refroot_manager = getManager();
    for(auto const &m : Referentiables::getManagers())
    {
        if(m.get() == refroot_manager) {
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
        m_refs.insert(ref);
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
    auto it = m_refs.find(ref);
    if_A(it != m_refs.end())
    {
        m_refs.erase(it);
        removeSpec(ref);
    }
}
