#pragma once

#include <set>

#include "referentiable.h"

namespace imajuscule
{
    class ReferentiableRoot : public Referentiable
    {
        friend class Globals;
        friend class ReferentiableManager<ReferentiableRoot>;
    public:
        static ReferentiableRoot * getInstance();

        void initialize();
        void addRef(Referentiable*);
        void removeRef(Referentiable*);
    protected:
        ReferentiableRoot(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName);

        DECL_PERSIST(ReferentiableRoot, Referentiable)
        VISITOR_HEADER_IMPL
        
        static ReferentiableRoot * g_instance;

        typedef std::set<Referentiable*> refs;
        refs m_refs;
    };
}

