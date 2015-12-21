//
//  RAII.hpp
//  model
//
//  Created by Olivier on 12/11/2015.
//  Copyright © 2015 Olivier. All rights reserved.
//

#pragma once
#include <functional>
#include "os.log.h"

namespace imajuscule {
    struct RAII
    {
        using F = std::function<void(void)>;
        
        F fDestructor;
        RAII(F const & fConstructor, F const & fDestructor):
        fDestructor(fDestructor)
        {
            fConstructor();
        }
        ~RAII()
        {
            fDestructor();
        }
    };

    struct inc_dec_RAII : public RAII
    {
        enum { minVal = 0 };
        inc_dec_RAII(int & i) : RAII([&i]() {
            A(i >= minVal);
            i++;
        }, [&i]() {
            i--;
            A(i >= minVal);
        }) {}
    };

}
