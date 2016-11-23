#pragma once

#include <type_traits>
#include <cstddef>

#include "os.log.h"

#include "visitable.h"

namespace imajuscule
{
    class Referentiable;
    template<class T = Referentiable>
    struct ref_shared_ptr : public NonCopyable {
        
        template<class> friend class ref_shared_ptr;
        
        ref_shared_ptr() : p(nullptr) {
        }
        
        // doesn't need to be explicit because it's an intrusive shared pointer
        ref_shared_ptr(T * ref) : p(ref) {
            if(p) {
                increment();
            }
        }

        ~ref_shared_ptr() {
            decrement();
        }

        void reset(T*o = nullptr) {
            if(p == o) {
                return;
            }
            decrement();
            p = o;
            if(p) {
                increment();
            }
        }

        ref_shared_ptr(ref_shared_ptr && o)
        : p(o.p) {
            o.p = nullptr;
        }

        template<class Y>
        ref_shared_ptr(ref_shared_ptr<Y> && o,
                       typename std::enable_if<std::is_convertible<Y*, T*>::value>::type = 0)
        : p(o.p) {
            o.p = nullptr;
        }
        
        ref_shared_ptr(ref_shared_ptr const & o)
        : p(o.p) {
            if(p) {
                increment();
            }
        }

        template<class Y>
        ref_shared_ptr(ref_shared_ptr<Y> const& o,
                       typename std::enable_if<std::is_convertible<Y*, T*>::value>::type = 0)
        : p(o.p) {
            if(p) {
                increment();
            }
        }
        
        
        ref_shared_ptr & operator = (const ref_shared_ptr & o) {
            reset(o.p);
            return *this;
        }

        ref_shared_ptr & operator = (ref_shared_ptr && o) {
            decrement();
            p = o.p;
            o.p = nullptr;
            return *this;
        }

        template<class Y>
        operator ref_shared_ptr<Y>() const {
            return {p};
        }
        
        template<class Y>
        operator Y *() const {
            return static_cast<Y*>(p);
        }
        
        void swap(ref_shared_ptr & o) {
            std::swap(p, o.p);
        }
        
        ref_shared_ptr & operator = (T * o) {
            reset(o);
            return *this;
        }

        template<class U>
        ref_shared_ptr<U> dynamic_as() const {
            if(std::is_same<U,T>::value) {
                return ref_shared_ptr<T>(*this);
            }
            auto d = dynamic_cast<U*>(p);
            if(!d) {
                return {};
            }
            return {d};
        }
        
        template<class U>
        ref_shared_ptr<U> static_as() const {
            auto s = static_cast<U*>(p);
            if(!s) {
                return {};
            }
            return {s};
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
        
        bool operator != ( std::nullptr_t ) const {
            return p != nullptr;
        }
        bool operator == ( std::nullptr_t ) const {
            return p == nullptr;
        }
        
        bool operator == (ref_shared_ptr const & o) const {
            return p == o.p;
        }
        bool operator != (ref_shared_ptr const & o) const {
            return p != o.p;
        }
        
        T * get() const { return p; }
        int32_t count() const { return p ? p->get_shared_counter() + 1 : 0; }

        // release ownership, decrements, but doesn't delete (the pointer will have no owner at the end)
        // this is used because the referentiable manager "owns the pointer until a first program shared_pointer
        // is created with it.
        T* forget() {
            A(count() == 1); // we are the only owner
            -- p->edit_shared_counter();
            A(count() == 0);
            
            auto ret = p;
            A(ret);
            p = nullptr;
            return ret;
        }
        
    private:
        T* p;
        
        bool decrement() {
            if(!p) {
                return false;
            }
            if(p->get_shared_counter() == 0) {
                p->deinstantiate();
                p = nullptr;
                return false;
            }
            -- p->edit_shared_counter();
            return true;
        }
        
        void increment() const {
            A(p);
            ++(p->edit_shared_counter());
        }
    };
    
    template<class T, class... Args>
    inline ref_shared_ptr<T>
    make_shared_ref(Args&&... args) {
        // todo optimize by allocation counter before object
        return ref_shared_ptr<T>(new T(std::forward<Args>(args)...));
    }
    
    template<class U, class T>
    ref_shared_ptr<U>  dynamic_pointer_cast(ref_shared_ptr<T> const & p) {
        return p.template dynamic_as<U>();
    }
    
    template<class U, class T>
    ref_shared_ptr<U>  static_pointer_cast(ref_shared_ptr<T> const & p) {
        return p.template static_as<U>();
    }
}

#define ref_shared_ptr ref_shared_ptr
