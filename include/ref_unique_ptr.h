#pragma once

#include <memory>
#include <functional>

namespace imajuscule
{
    template<class T>
    struct ref_unique_ptr : public std::unique_ptr<T, std::function<void(T*)>> {
        ref_unique_ptr(T * ref = 0) :
        std::unique_ptr<T, std::function<void(T*)>>(ref, [](T*r) {
            r->deinstantiate();
        })
        {}
    };
    
    template<class T, class... Args>
    inline ref_unique_ptr<T>
    make_unique_ref(Args&&... args)
    {
        return ref_unique_ptr<T>(new T(_VSTD::forward<Args>(args)...));
    }
    
    template<class T>
    struct ref_shared_ptr : public std::shared_ptr<T> {
        ref_shared_ptr(T * ref = 0) :
        std::shared_ptr<T>(ref, [](T*r) {
            r->deinstantiate();
        })
        {}
    };
    
    template<class T, class... Args>
    inline ref_shared_ptr<T>
    make_shared_ref(Args&&... args)
    {
        return ref_shared_ptr<T>(new T(_VSTD::forward<Args>(args)...));
    }
    
}
