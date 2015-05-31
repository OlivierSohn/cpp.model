#pragma once

#include "referentiable.h"

#include "referentiable.manager.h"

#include <map>

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
        PERSISTABLE_VISITOR_HEADER_IMPL
        
        static ReferentiableRoot * g_instance;

        typedef std::map<Referentiable*,LINK(Referentiable)> refs;
        refs m_refs;
    };
}

