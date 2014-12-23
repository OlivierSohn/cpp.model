#pragma once
#include "positionable.h"
#include "wmoperation.h"
#include "paramset.filepath.h"
#include "Wire.h"

class WireModel;

namespace imajuscule
{
    class Body;

    class SpecWM : public Positionable, public WMOperation
    {
    public:
        SpecWM();
        virtual ~SpecWM();

        // TODO think about API
        /*
        void addPointToPolyLine();
        void addPointToCurve();
        void changePointCoordinates();
        */
        
        void doUpdate() override;

        PERSISTABLE_VISITOR_HEADER_IMPL
    private:
        FilePath m_filePath;
    };
}