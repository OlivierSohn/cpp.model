
#pragma once

namespace imajuscule
{
#define PERSISTABLE_VISITOR_PURE_VIRTUAL virtual void accept(PersistableVisitor &) = 0;

#define PERSISTABLE_VISITOR_HEADER_IMPL virtual void accept(PersistableVisitor & vtor) override\
    {\
        vtor.Visit(this);\
    }

    // forward declarations of Persistables defined in gl.view
    class FormulaBase;
    class AnimationBase;
    class ParamBase;
    class ParamSet;
    class Joint;
    class SimSTAIBody;
    class RasterSTAIBody;
    class SpecWM;
    class RoundedWM;
    class WMGeoOp;
    class RasterizeOp;
    class STAIRenderOp;
    class ShrinkOp;
    class STAIRasterOp;
    class STAISimOp;
    class Positionable;
    class EvtB;

    class PersistableVisitor
    {
    public:
        virtual ~PersistableVisitor(){}

        // TODO PersistableVisitor: subclass in gl.view and add "Visit" pure virtual methods
        virtual void Visit(AnimationBase*) = 0;
        virtual void Visit(ParamBase*) = 0;
        virtual void Visit(ParamSet*) = 0;
        virtual void Visit(Joint*) = 0;
        virtual void Visit(RasterSTAIBody*) = 0;
        virtual void Visit(SimSTAIBody*) = 0;
        virtual void Visit(SpecWM*) = 0;
        virtual void Visit(RoundedWM*) = 0;
        virtual void Visit(WMGeoOp*) = 0;
        virtual void Visit(RasterizeOp*) = 0;
        virtual void Visit(STAIRenderOp*) = 0;
        virtual void Visit(ShrinkOp*) = 0;
        virtual void Visit(STAIRasterOp*) = 0;
        virtual void Visit(STAISimOp*) = 0;
        virtual void Visit(Positionable*) = 0;
        virtual void Visit(FormulaBase*) = 0;
        virtual void Visit(EvtB*) = 0;

    protected:
        PersistableVisitor() {}
    };
}
