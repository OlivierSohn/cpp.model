#include "wmoperation.h"
#include "WireBuildingModel.h"

using namespace imajuscule;

WMOperation::WMOperation() :
m_wireModel(NULL),
m_pWsb(NULL)
{
}

WMOperation::~WMOperation()
{
    if (m_pWsb)
        delete m_pWsb;

    if (m_wireModel)
        delete m_wireModel;
}

WireModel * WMOperation::NewModel()
{
    if (m_wireModel)
    {
        delete m_wireModel;
        m_wireModel = NULL;
    }

    if (m_pWsb)
    {
        delete m_pWsb;
        m_pWsb = NULL;
    }

    m_wireModel = new WireBuildingModel();

    return m_wireModel;
}

void WMOperation::PublishModel()
{
    if (m_wireModel)
    {
        if (!m_pWsb)
            m_pWsb = new WireSetBounds();

        m_wireModel->GetWires(*m_pWsb);
    }
    else
    {
        LG(ERR, "WMOperation::PublishModel : model is NULL");
        assert(0);

        if (m_pWsb)
        {
            delete m_pWsb;
            m_pWsb = NULL;
        }
    }
}

WireSetBounds * WMOperation::getWireSetBounds()
{
    return m_pWsb;
}