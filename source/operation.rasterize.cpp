#include "operation.rasterize.h"
#include "Rasterizer/WireRasterizer.h"

using namespace imajuscule;

RasterizeOp::RasterizeOp(GeoOp&GeoOpRef) :
Updatable(),
m_geoOp(GeoOpRef),
m_pRaster(NULL)
{
    addSpec(&m_geoOp);
    addSpec(&m_rasterize);
    addSpec(&m_extrude);
}

RasterizeOp::~RasterizeOp()
{
    removeSpec(&m_geoOp);
    removeSpec(&m_rasterize);
    removeSpec(&m_extrude);
}

void RasterizeOp::doUpdate()
{
    if (m_pRaster)
        delete m_pRaster;
    m_pRaster = new Raster();

    int iAxis, length;
    m_extrude.getParams(iAxis, length);

    float gridSize = 0.f;
    bool bAsVoxels;
    m_rasterize.getParams(gridSize, bAsVoxels);
    WireSetsBounds wssb;
    m_geoOp.getWireSetsBounds(wssb);

    WireSetsBounds_Iterator it = wssb.begin();
    WireSetsBounds_Iterator end = wssb.end();

    for (; it != end; ++it)
    {
        WireRasterizer::Rasterize(*it, *m_pRaster, bAsVoxels);
    }

    if (length > 0)
    {
        m_pRaster->Extrude(length, IntToTubeOrientation(iAxis));
    }
}