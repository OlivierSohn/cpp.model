#include "operation.render.h"

using namespace imajuscule;

RenderOp::RenderOp(RasterizeOp&OpRef) :
Updatable(),
m_rasterizeOp(OpRef)
{
    addSpec(&m_rasterizeOp);
    addSpec(&m_color);
    addSpec(&m_shrink);
}

RenderOp::~RenderOp()
{
    removeSpec(&m_rasterizeOp);
    removeSpec(&m_color);
    removeSpec(&m_shrink);
}

void RenderOp::doUpdate()
{
    float alpha;
    m_color.getParams(alpha);

    float shrink;
    m_shrink.getParams(shrink);

    Raster * raster = m_rasterizeOp.getRaster();
    if (raster)
    {
        // TODO
        // convert to vector of floats
    }
}