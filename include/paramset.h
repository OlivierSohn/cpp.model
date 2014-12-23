#pragma once

#include "param.h"
#include "updatable.h"
#include <vector>

namespace imajuscule
{

    class ParamSet : public Updatable
    {
    public:
        typedef std::vector< ParamBase > paramsInSet;
        ParamSet(const char* paramSetName, paramsInSet & vParams);
        virtual ~ParamSet();

        const char * name() const;

        ParamBase & getParam(unsigned int index);

        void doUpdate() override;

        PERSISTABLE_VISITOR_HEADER_IMPL

    private:
        const char * m_name;
        paramsInSet m_params;
    };
}