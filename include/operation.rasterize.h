#pragma once

#include "updatable.h"
#include "operation.geo.h"
#include "Rasterizer/Raster.h"
#include "paramset.rasterize.h"
#include "paramset.extrude.h"

namespace imajuscule
{
    class RasterizeOp : public Updatable
    {
    public:
        RasterizeOp(GeoOp&);
        virtual ~RasterizeOp();

        void doUpdate() override;

        PERSISTABLE_VISITOR_HEADER_IMPL
    private:
        GeoOp& m_geoOp;
        Raster * m_pRaster;

        Rasterize m_rasterize;
        Extrude m_extrude;
    };
}