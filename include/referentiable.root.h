#pragma once

#include <map>

#include "referentiable.h"

namespace imajuscule
{
    class ReferentiableRoot : public Referentiable
    {
        friend class ReferentiableManager<ReferentiableRoot>;
    public:
        static ReferentiableRoot * getInstance();
        void addRef(Referentiable*);
        void removeRef(Referentiable*);
    protected:
        ReferentiableRoot(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName);

        virtual ~ReferentiableRoot();

        DECL_PERSIST(ReferentiableRoot, Referentiable)
        VISITOR_HEADER_IMPL
        
        static ReferentiableRoot * g_instance;

        typedef std::map<Referentiable*,LINK(Referentiable)> refs;
        refs m_refs;
    };
}

