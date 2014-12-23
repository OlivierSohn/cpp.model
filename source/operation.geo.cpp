#include "operation.geo.h"

using namespace imajuscule;

GeoOp::GeoOp()
{

}
GeoOp::~GeoOp()
{
    cplxOperations::iterator it = m_operations.begin();
    cplxOperations::iterator end = m_operations.end();
    for (; it != end; ++it)
    {
        removeSpec(it->second);
        delete it->second;
        delete it->first;
    }
}

void GeoOp::addRoundedWMOperation()
{
    SpecWM * swm = new SpecWM();
    RoundedWM * rwm = new RoundedWM(swm);
    m_operations.push_back(cplxOperations::value_type(swm, rwm));

    addSpec(rwm);
}

void GeoOp::getWireSetsBounds(WireSetsBounds & wsb)
{
    wsb = m_wssb;
}

void GeoOp::doUpdate()
{
    m_wssb.clear();

    cplxOperations::iterator it = m_operations.begin();
    cplxOperations::iterator end = m_operations.end();
    for (; it != end; ++it)
    {
        WireSetBounds * wsb = it->second->getWireSetBounds();
        if (wsb)
            m_wssb.push_back(*wsb);
    }
}
