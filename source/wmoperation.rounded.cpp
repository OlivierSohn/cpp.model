#include "wmoperation.rounded.h"
#include "wmoperation.spec.h"
#include "WireBuildingModel.h"

using namespace imajuscule;

RoundedWM::RoundedWM(SpecWM * spec) :
WMOperation()
{
    this->addSpec(&m_rounding);
    if ( m_spec )
        this->addSpec(m_spec);
}

RoundedWM::~RoundedWM()
{
    this->removeSpec(&m_rounding);
    if ( m_spec )
        this->removeSpec(m_spec);
}

void RoundedWM::doUpdate()
{
    WireModel * model = NewModel();

    WireSetBounds * wsb = NULL;
    if (m_spec && (wsb = m_spec->getWireSetBounds()))
    {
        model->Include(*wsb, 1.0f);

        float RoundingRatio, RoundingStrength;
        m_rounding.getRoundings(RoundingRatio, RoundingStrength);

        model->SetConcreteParam("RR", RoundingRatio);
        model->SetConcreteParam("RS", RoundingStrength);

        PublishModel();
    }
}
