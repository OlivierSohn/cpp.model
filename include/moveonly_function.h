#pragma once

// from https://gist.github.com/vmilea/5815777

// this header was not available so I hardcoded the function type
//#include "meta/FunctionTraits.h"
// if needed it is available here :
// https://github.com/vmilea/CppAsync/blob/master/CppAsync/util/FunctionTraits.h

#include <utility>
#include <stdexcept>
#include <functional>
#include <string>
#include <cassert>

namespace imajuscule {
    
    template <typename F>
    class MoveOnCopyAdapter
    {
    public:
        MoveOnCopyAdapter(F&& f)
        : mF(std::move(f)) { }
        
        MoveOnCopyAdapter(MoveOnCopyAdapter&& other)
        : mF(std::move(other.mF)) { }
        
        MoveOnCopyAdapter& operator=(MoveOnCopyAdapter&& other)
        {
            mF = std::move(other.mF);
            
            return *this;
        }
        
        MoveOnCopyAdapter(const MoveOnCopyAdapter& other)
        : mF(static_cast<F&&>(const_cast<F&>(other.mF)))
        {
            throw std::logic_error("not copyable");
        }
        
        MoveOnCopyAdapter& operator=(const MoveOnCopyAdapter& other)
        {
            throw std::logic_error("not copyable");
            
        }
        
        template <typename... Args>
        auto operator()(Args&&... args)
        -> typename std::result_of<F (Args...)>::type
        {
            using namespace std;
            return mF(std::forward<Args>(args)...);
        }
        
    private:
        F mF;
    };
    
    template <typename F>
    auto asFunction(F&& f)
//    -> std::function<typename ut::FunctionTraits<F>::signature_type>
    -> std::function<void(void)>
    {
        static_assert (std::is_rvalue_reference<F&&>::value, "needs rvalue");
        
        return MoveOnCopyAdapter<F>(std::move(f));
    }

}
