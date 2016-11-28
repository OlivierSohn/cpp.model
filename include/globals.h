#pragma once

#include <functional>

#include <vector>

#include "referentiable.h"
#include "referentiable.manager.h"

namespace imajuscule {
    using reseter = std::function<void(void)>;
    
    class GlobalsImpl {
    public:
        void addReseter(reseter);
        bool isResetting() const;
        
        static GlobalsImpl * getInstance();
        static void resetInstance();
    private:
        
        void reset();
        ~GlobalsImpl();
        
        static GlobalsImpl * instance;
        bool resetting = false;
        std::vector<std::function<void(void)>> reseters;
    };
    
    class Globals {
    public:
        
        template<typename T, typename... Args>
        static T* ptr(T *& p, Args&&... args) {
            if(p) {
                return p;
            }
            make_ptr(p, std::forward<Args>(args)...);
            return p;
        }
        
        template<typename T, typename... Args>
        static void make_ptr(T *& p, Args&&... args) {
            if( GlobalsImpl::getInstance()->isResetting() ) {
                delete p;
                p=0;
                return;
            }
            p = new T(args...);
            add_ptr(p);
        }
        
        template<typename T, typename... Args>
        static T* ref(intrusive_ptr<T>& p, Args&&... args) {
            if(p) {
                return p.get();
            }
            make_ref(p, std::forward<Args>(args)...);
            return p.get();
        }
        
        template<typename T, typename... Args>
        static void make_ref(intrusive_ptr<T>& p, Args&&... args) {
            if( GlobalsImpl::getInstance()->isResetting() ) {
                LG(ERR, "GlobalsImpl is resetting, cannot make ref");
                if(p) {
                    p->deinstantiate();
                }
                p=0;
                return;
            }
            auto rm = ReferentiableManager<T>::getInstance();
            if(!rm) {
                LG(ERR, "referentiable manager is 0, cannot make ref");
                if(p) {
                    p->deinstantiate();
                }
                p=0;
                return;
            }
            p.reset(rm->New().release());
            add_ref(p);
        }

        static void reset() {
            GlobalsImpl::resetInstance();
        }
    private:
        
        // can delete and reset the pointer to zero
        template<typename T>
        static void add_ptr(T *& p) {
            if(do_add_ptr(p)) {
                return;
            }
            delete p;
            p=0;
        }
        
        // can delete and reset the pointer to zero
        template<typename T>
        static void add_ref(intrusive_ptr<T>& p) {
            if( do_add_ref(p) ) {
                return;
            }
            p->deinstantiate();
            p=0;
        }

        template<typename T>
        static bool do_add_ptr(T*& p) {
            if( GlobalsImpl::getInstance()->isResetting() ) {
                LG(ERR, "GlobalsImpl is resetting, cannot add pointer");
                return false;
            }
            GlobalsImpl::getInstance()->addReseter([&p](){
                delete p;
                p=0;
            });
            return true;
        }
        
        template<typename T>
        static bool do_add_ref(intrusive_ptr<T>& p) {
            if( GlobalsImpl::getInstance()->isResetting() ) {
                LG(ERR, "GlobalsImpl is resetting, cannot add referentiable");
                return false;
            }
            GlobalsImpl::getInstance()->addReseter([&p](){
                p.reset();
            });
            return true;
        }
    };
}
