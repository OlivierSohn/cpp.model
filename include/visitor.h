
#pragma once

namespace imajuscule
{
    class PathSuite;
    class FormulaBase;
    class Referentiable;
    class ParamBase;
    class ParamSet;
    class Joint;
    class KinChain;
    class SimSTAIBody;
	class RasterSTAIBody;
	class EffectSTAIBody;
	class SpecWM;
    class RoundedWM;
    class WMGeoOp;
    class RasterizeOp;
    class STAIRenderOp;
    class ShrinkOp;
    class ScaleOp;
    class STAIRasterOp;
    class STAISimOp;
    class Positionable;
    class EvtB;
    class Player;
    class ReferentiableManagerBase;
    class Shot;
    class CameraBase;
    class MotionLayer;
    class CurveTraversal;
    class WireModel;
    class WireTextModel;
    class Playable;
    class ExactVolume;
    class Human;
    class HumanControl;
    class MixerBase;
    class GenericUpdatable;

    class Visitor
    {
    public:
        virtual ~Visitor(){}

        virtual void Visit(PathSuite*) = 0;
        virtual void Visit(Referentiable*) = 0;
        virtual void Visit(ParamBase*) = 0;
        virtual void Visit(ParamSet*) = 0;
        virtual void Visit(Joint*) = 0;
        virtual void Visit(KinChain*) = 0;
        virtual void Visit(RasterSTAIBody*) = 0;
		virtual void Visit(SimSTAIBody*) = 0;
		virtual void Visit(EffectSTAIBody*) = 0;
		virtual void Visit(SpecWM*) = 0;
        virtual void Visit(RoundedWM*) = 0;
        virtual void Visit(WMGeoOp*) = 0;
        virtual void Visit(RasterizeOp*) = 0;
        virtual void Visit(STAIRenderOp*) = 0;
        virtual void Visit(ShrinkOp*) = 0;
        virtual void Visit(ScaleOp*) = 0;
        virtual void Visit(STAIRasterOp*) = 0;
		virtual void Visit(STAISimOp*) = 0;
		virtual void Visit(Positionable*) = 0;
        virtual void Visit(FormulaBase*) = 0;
        virtual void Visit(EvtB*) = 0;
        virtual void Visit(Player*) = 0;
        virtual void Visit(ReferentiableManagerBase*) = 0;
        virtual void Visit(Shot*) = 0;
        virtual void Visit(MotionLayer*) = 0;
        virtual void Visit(CurveTraversal*) = 0;
        virtual void Visit(CameraBase*) = 0;
        virtual void Visit(WireModel*) = 0;
        virtual void Visit(WireTextModel*) = 0;
        virtual void Visit(ExactVolume*) = 0;
        virtual void Visit(Playable*) {};
        virtual void Visit(Human*) = 0;
        virtual void Visit(HumanControl*) {};
        virtual void Visit(MixerBase*) {};
        virtual void Visit(GenericUpdatable*) {};

    protected:
        Visitor() {}
    };
}
