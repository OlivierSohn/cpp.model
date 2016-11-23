#include "os.log.h"

#include "globals.h"

#include "referentiables.h"
#include "referentiable.manager.h"

#include "referentiable.root.h"

const int KEY_TARGETS = -110;

using namespace imajuscule;

ref_shared_ptr<ReferentiableRoot> ReferentiableRoot::g_instance;

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
        if(!m || m == refroot_manager) {
            continue;
        }
        for (auto vi : m->traverse() ) {
            addSpec(vi);
        }
        m->observable().Register(ReferentiableManagerBase::Event::RFTBL_ADD,
                                  [this](Referentiable* r){
                                      if(!dynamic_cast<ReferentiableRoot*>(r)) {
                                          addSpec(r);
                                      }
                                  });
    }
}
