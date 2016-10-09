#include "globals.h"

namespace imajuscule {
    GlobalsImpl * GlobalsImpl::instance = 0;
    
    GlobalsImpl * GlobalsImpl::getInstance() {
        if(!instance) {
            instance = new GlobalsImpl();
        }
        return instance;
    }
    void GlobalsImpl::resetInstance() {
        if(instance) {
            delete instance;
            instance = 0;
        }
    }
    
    GlobalsImpl::~GlobalsImpl() {
        reset();
    }
    
    bool GlobalsImpl::isResetting() const {
        return resetting;
    }
    
    void GlobalsImpl::reset() {
        A(!resetting);
        resetting = true;
        
        for(auto it = reseters.rbegin(); it!= reseters.rend(); ++it) {
            A(resetting);
            (*it)();
            A(resetting);
        }
        reseters.clear();
        A(resetting);
        resetting = false;
    }
    
    void GlobalsImpl::addReseter(reseter r) {
        reseters.emplace_back(std::move(r));
    }
}
