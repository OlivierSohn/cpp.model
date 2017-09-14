
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
        Assert(!resetting);
        resetting = true;
        
        for(auto it = reseters.rbegin(); it!= reseters.rend(); ++it) {
            Assert(resetting);
            (*it)();
            Assert(resetting);
        }
        reseters.clear();
        Assert(resetting);
        resetting = false;
    }
    
    void GlobalsImpl::addReseter(reseter r) {
        reseters.emplace_back(std::move(r));
    }
}
