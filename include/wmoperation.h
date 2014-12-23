#pragma once
#include "positionable.h"
#include "paramset.filepath.h"
#include "Wire.h"

class WireModel;

namespace imajuscule
{
    class WMOperation 
    {
    public:
        WMOperation();
        virtual ~WMOperation();

        WireSetBounds * getWireSetBounds();

    protected:
        WireModel * NewModel();
        void PublishModel();

    private:
        WireModel * m_wireModel;
        WireSetBounds * m_pWsb;
    };
}