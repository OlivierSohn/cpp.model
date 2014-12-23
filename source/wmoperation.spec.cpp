#include "wmoperation.spec.h"
#include "WireBuildingModel.h"

using namespace imajuscule;

SpecWM::SpecWM() :
Positionable(NULL),
WMOperation()
{
    this->addSpec(&m_filePath);
}

SpecWM::~SpecWM()
{
    this->removeSpec(&m_filePath);
}

void SpecWM::doUpdate()
{
    WireModel * model = NewModel();

    std::string file;
    m_filePath.getPath(file);

    eResult resLoad = model->LoadWires(file);
    if (resLoad == ILE_SUCCESS)
    {
        PublishModel();
    }
    else
    {
        LG(WARN, "SimBuildingAppendPositions : load .mdl file error %d", resLoad);
    }
}