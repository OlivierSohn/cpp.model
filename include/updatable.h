
#pragma once

#include "observable.h"
#include <list>
#include <set>

#include "visitor.h"

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

    protected:
        Updatable();

        virtual bool doUpdate() { return hasNewContentForUpdate(); };

    private:
        specs m_specs;

        bool isConsistent() const;
        bool isSpec(spec item) const;        
    };
}
