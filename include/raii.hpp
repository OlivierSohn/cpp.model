//
//  RAII.hpp
//  model
//
//  Created by Olivier on 12/11/2015.
//  Copyright © 2015 Olivier. All rights reserved.
//

#pragma once
#include <functional>

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
}