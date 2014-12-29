
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
        
        // check for difference to know if you have up-to-date data
        unsigned int stamp();

        PERSISTABLE_VISITOR_PURE_VIRTUAL

    protected:
        Updatable();

        virtual void doUpdate() = 0;

    private:
        void onObservedChanged() override;

        bool isConsistent() const;
        bool isSpec(spec item) const;
        
        bool m_bOneObservedChanged;
        unsigned int m_stamp;

        specs m_specs;
    };
}
