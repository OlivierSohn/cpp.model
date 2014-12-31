
#pragma once

#include "observable.h"
#include <list>
#include <vector>
#include <memory>
#include <map>

#include "visitor.persistable.h"

namespace imajuscule
{
    class Updatable;
    typedef Updatable * spec;
    typedef std::list< spec > specs;
    
    class Updatable : public Observable
    {
    public:
        virtual ~Updatable();

        void Update();

        void addSpec(spec);
        void removeSpec(spec);

        void traverseSpecs(specs::iterator & begin, specs::iterator & end);
        
        PERSISTABLE_VISITOR_PURE_VIRTUAL

    protected:
        Updatable();

        virtual bool doUpdate() { return hasNewContentForUpdate(); };

    private:
        bool isConsistent() const;
        bool isSpec(spec item) const;
        
        specs m_specs;
    };
}
