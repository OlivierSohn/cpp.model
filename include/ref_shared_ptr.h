#pragma once

#include "os.log.h"

#include "referentiable.h"

namespace imajuscule
{
    template<class T = Referentiable>
    struct ref_shared_ptr : public NonCopyable {
        ref_shared_ptr(T * ref = 0) : p(ref), c(ref?new int32_t:nullptr) {
            initialize();
        }

        ~ref_shared_ptr() {
            decrement();
        }

        void reset() {
            if(decrement()) {
                p = nullptr;
            }
            c = nullptr;
        }
        
        void reset(T*o) {
            if(decrement()) {
                c = nullptr;
            }
            p = o;
            if(!c) {
                c = new int32_t;
            }
            initialize();
        }

        ref_shared_ptr(ref_shared_ptr && o) {
            p = o.p;
            c = o.c;
            o.p = nullptr;
            o.c = nullptr;
        }
        
        ref_shared_ptr(ref_shared_ptr const & o) {
            p = o.p;
            c = o.c;
            if(p) {
                increment();
            }
        }
        
        ref_shared_ptr & operator = (const ref_shared_ptr & o) {
            if(p != o.p) {
                decrement();
                p = o.p;
                c = o.c;
                if(p) {
                    increment();
                }
            }
            return *this;
        }

        ref_shared_ptr & operator = (ref_shared_ptr && o) {
            decrement();
            p = o.p;
            c = o.c;
            o.p = nullptr;
            o.c = nullptr;
            return *this;
        }

        void swap(ref_shared_ptr<T> & o) {
            std::swap(p, o.p);
            std::swap(c, o.c);
        }
        
        ref_shared_ptr & operator = (T * o) {
            reset(o);
            return *this;
        }
        
        explicit operator T*() const { return p; }
        operator bool() const { return static_cast<bool>(p); }
        T& operator*() const { return *p; }
        T* operator -> () const { return p; }
        
        bool operator == (T*o) const {
            return p == o;
        }
        bool operator != (T*o) const {
            return p != o;
        }
        
        bool operator == (ref_shared_ptr const & o) const {
            return p == o.p;
        }
        bool operator != (ref_shared_ptr const & o) const {
            return p != o.p;
        }
        
        T * get() const { return p; }
        int32_t count() const { return p ? *c + 1 : 0; }

    private:
        int32_t * c;
        T* p;
        
        bool decrement() {
            if(!p) {
                return false;
            }
            A(c);
            auto & count = *c;
            if(count == 0) {
                p->deinstantiate();
                p = nullptr;
                delete c;
                c = nullptr;
                return false;
            }
            -- count;
            return true;
        }
        
        void increment() {
            ++(*c);
        }
        
        void initialize() {
            if(p) {
                *c = 0;
            }
        }
    };
    
    template<class T, class... Args>
    inline ref_shared_ptr<T>
    make_shared_ref(Args&&... args) {
        // todo optimize by allocation counter before object
        return ref_shared_ptr<T>(new T(std::forward<Args>(args)...));
    }
    
}
