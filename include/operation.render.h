#pragma once

#include "updatable.h"
#include "operation.rasterize.h"
#include "paramset.shrink.h"
#include "paramset.color.h"

namespace imajuscule
{
    class RenderOp : public Updatable
    {
    public:
        RenderOp(RasterizeOp&);
        virtual ~RenderOp();

        void doUpdate() override;

        PERSISTABLE_VISITOR_HEADER_IMPL
    private:
        RasterizeOp& m_rasterizeOp;
        Shrink m_shrink;
        Color m_color;
    };
}