
#pragma once

#include "persistable.h"
#include <list>
#include <vector>
#include <memory>
#include <map>

namespace imajuscule
{
    class Updatable;
    typedef Updatable * spec;
    typedef std::list< spec > specs;
    
    class Updatable : public Persistable
    {
    public:
        virtual ~Updatable();

        void Update();

        void addSpec(spec);
        void removeSpec(spec);

        void traverseSpecs(specs::iterator & begin, specs::iterator & end);
        
        // check for difference to know if you have up-to-date data
        unsigned int stamp();
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
