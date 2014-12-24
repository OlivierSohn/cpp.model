
#pragma once

#include "persistable.h"
#include <list>
#include <memory>

namespace imajuscule
{

    class Updatable : public Persistable
    {
    public:
        typedef Updatable * spec;

        virtual ~Updatable();

        void Update();

        void addSpec(spec);
        void removeSpec(spec);

        // check for difference to know if you have up-to-date data
        unsigned int stamp();
    protected:
        Updatable();

        virtual void doUpdate() = 0;

        typedef std::list< spec > specs;
    private:
        void onObservedChanged() override;

        bool isConsistent() const;
        bool isSpec(spec item) const;

        bool m_bOneObservedChanged;
        unsigned int m_stamp;

        specs m_specs;
    };
}
