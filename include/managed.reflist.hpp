#pragma once

#include <list>
#include <vector>
#include "referentiable.h"
#include "referentiable.cmd.list.h"

namespace imajuscule
{
    using regs = std::vector < FunctionInfo< Referentiable::Event >>;
    using vregs = std::vector < regs >;
    
#define MANAGED_REF_LIST template <typename Owner, class T, void (Owner::*Add)(T*), void (Owner::*Remove)(T*)>
    MANAGED_REF_LIST
    class ManagedRefList {
        friend Owner;
        
        Owner * owner;
        vregs m_regs;
        using listT = std::vector<LINK(T)>;
        using listIterator = typename listT::iterator;
        listT list;

        void addInternal( T* );
        void removeInternal( T* );
        
        using cmd = RefAttrListCmd < Owner, T, Add, Remove >;
        void remove(vregs::iterator & reg, listIterator & layer);
    
    public:
        using Owner_T = Owner;
        using Element_T = T;
        ManagedRefList(Owner * owner) : owner(owner) {}
        ~ManagedRefList();

        Owner_T & editOwner() { return *owner; }
        void add(T*);
        void remove(T*);
        void set(std::vector<T*> &&);
        bool has(T const *) const;
        int size() const;
        bool empty() const;
        void clear();
        T const * get(int) const;
        T * edit(int);
        T& operator[] (const int index) { return *list[index]; }
        const T& operator[] (const int index) const { return *list[index]; }
        
        listT const & get() const { return list; }
        listT & edit() { return list; }
    };
}

#include "managed.reflist.hxx"
