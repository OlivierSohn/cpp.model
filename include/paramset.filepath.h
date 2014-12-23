#pragma once

#include "paramset.h"

namespace imajuscule
{
    class FilePath : public ParamSet
    {
    public:
        FilePath();
        virtual ~FilePath();

        void getPath(std::string & path);
    };
}