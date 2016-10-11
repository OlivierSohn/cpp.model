#pragma once

#include <list>
#include <vector>

#include "referentiable.h"
#include "referentiable.cmd.list.h"

namespace imajuscule
{
    template <class T>
    struct VECTOR_ELEMENT {
        using type = ref_unique_ptr<T>;
    };
    template <>
    struct VECTOR_ELEMENT<MotionLayer> {
        using type = MotionLayer *;
    };
    
    using regs = std::vector < FunctionInfo< Referentiable::Event >>;
    using vregs = std::vector < regs >;
    
#define MANAGED_REF_LIST template <typename Owner, class T, void (Owner::*Add)(T*), bool (Owner::*Remove)(T*)>
    MANAGED_REF_LIST
    class ManagedRefList {
        friend Owner;
        
        Owner * owner;
        vregs m_regs;
        using ELEM_T = typename VECTOR_ELEMENT<T>::type;
        using listT = std::vector<ELEM_T>;
        using listIterator = typename listT::iterator;
        listT list;

        void addInternal( T* );
        bool removeInternal( T* );
        
        using cmd = RefAttrListCmd < Owner, T, Add, Remove >;
        void remove(vregs::iterator & reg, listIterator & layer);
    
    public:
        using Owner_T = Owner;
        using Element_T = T;
        
        ManagedRefList(Owner * owner) : owner(owner) {}
        ~ManagedRefList();

        Owner_T & editOwner() { return *owner; }

        template<class U> // U is T or subclass
        void add(ref_unique_ptr<U> ptr) {
            bool found;
            cmd::ManageAttr(*owner, ptr.release(), cmd::Type::TYPE_ADD, found);
        }
        
        // TODO remove once we figured out a way to handle shared MotionLayer
        void add(T * ptr) {
            bool found;
            cmd::ManageAttr(*owner, ptr, cmd::Type::TYPE_ADD, found);
        }
        bool remove(T*);
        void set(listT &&);
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
