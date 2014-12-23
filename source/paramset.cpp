#include "paramset.h"

using namespace imajuscule;

ParamSet::ParamSet(const char* paramSetName, std::vector< ParamBase > & vParams) :
Updatable(),
m_name(paramSetName)
{
    m_params.swap(vParams);

    paramsInSet::iterator it = m_params.begin();
    paramsInSet::iterator end = m_params.end();
    for (; it != end; ++it)
    {
        (*it).addObserver(this);
        this->addSpec(&(*it));
    }

}

ParamSet::~ParamSet()
{
    paramsInSet::iterator it = m_params.begin();
    paramsInSet::iterator end = m_params.end();
    for (; it != end; ++it)
    {
        this->removeSpec(&(*it));
        (*it).removeObserver(this);
    }
}

const char * ParamSet::name() const
{
    return m_name;
}