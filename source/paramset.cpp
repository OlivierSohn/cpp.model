#include "paramset.h"

using namespace imajuscule;

ParamSet::ParamSet(const char* paramSetName, paramsInSet & vParams) :
Updatable(),
m_name(paramSetName)
{
    m_params.swap(vParams);

    paramsInSet::iterator it = m_params.begin();
    paramsInSet::iterator end = m_params.end();
    for (; it != end; ++it)
    {
        this->addSpec(*it);
    }

}

ParamSet::~ParamSet()
{
    paramsInSet::iterator it = m_params.begin();
    paramsInSet::iterator end = m_params.end();
    for (; it != end; ++it)
    {
        this->removeSpec(*it);
        delete(*it);
    }
}

const char * ParamSet::name() const
{
    return m_name;
}


ParamBase & ParamSet::getParam(unsigned int index)
{
    return *m_params.at(index);
}

void ParamSet::doUpdate()
{}