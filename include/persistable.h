
#pragma once

namespace imajuscule
{

#define PERSISTABLE_VISITOR_PURE_VIRTUAL virtual void accept(PersistableVisitor &) = 0;

#define PERSISTABLE_VISITOR_HEADER_IMPL virtual void accept(PersistableVisitor & vtor) override\
    {\
        vtor.Visit(this);\
    }

    // forward declarations of Persistables defined in gl.view
    class ParamBase;
    class ParamSet;
    class Joint;
    class Body;
    class SpecWM;
    class RoundedWM;
    class GeoOp;
    class RasterizeOp;
    class RenderOp;

    class PersistableVisitor
    {
    public:
        virtual ~PersistableVisitor(){}

        // TODO subclass in gl.view and add "Visit" pure virtual methods
        virtual void Visit(ParamBase*) = 0;
        virtual void Visit(ParamSet*) = 0;
        virtual void Visit(Joint*) = 0;
        virtual void Visit(Body*) = 0;
        virtual void Visit(SpecWM*) = 0;
        virtual void Visit(RoundedWM*) = 0;
        virtual void Visit(GeoOp*) = 0;
        virtual void Visit(RasterizeOp*) = 0;
        virtual void Visit(RenderOp*) = 0;

    protected:
        PersistableVisitor() {}
    };

    class Persistable
    {
    public:
        virtual ~Persistable();

        PERSISTABLE_VISITOR_PURE_VIRTUAL

    protected:
        Persistable();
    };
}
