#include "param.h"

using namespace imajuscule;

ParamBase::ParamBase(const char* paramName) :
Updatable(),
m_name(paramName)
{
}
ParamBase::~ParamBase()
{}

const char * ParamBase::name() const
{
    return m_name;
}

template <typename T>
Param<T>::Param(const char* paramName, T initialValue) :
ParamBase(paramName),
m_val(initialValue)
{
}

template <typename T>
Param<T>::~Param()
{}

template <typename T>
void Param<T>::setValue(T val)
{
    if (val != m_val)
    {
        m_val = val;
        notifyObservers();
    }
}

template <typename T>
T Param<T>::GetValue() const
{
    return m_val;
}
template <typename T>
void Param<T>::GetValue(T&oVal) const
{
    oVal = m_val;
}

////////////////////////////////////////////////////////////////////
// instantiate the templates (add some if some are unresolved)
////////////////////////////////////////////////////////////////////

#include <string>

template class Param < float >;
template class Param < int >;
template class Param < bool >;
template class Param < std::string >;