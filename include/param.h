#pragma once

#include "updatable.h"

namespace imajuscule
{
    class ParamBase : public Updatable
    {
    public:
        const char * name() const;
        virtual ~ParamBase();
    protected:
        ParamBase(const char* paramName);

        PERSISTABLE_VISITOR_HEADER_IMPL

            // TODO animated parameters should override this implementation
            void doUpdate() override  {}
    private:
        const char * m_name;
    };

    template <typename T> class Param : public ParamBase
    {
    public:
        Param(const char* paramName, T initialValue);
        virtual ~Param();

        void setValue(T);

        T GetValue() const;
        void GetValue(T&) const;


    private:
        T m_val;
    };
}