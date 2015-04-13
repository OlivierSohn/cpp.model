#pragma once

#include <vector>
#include <stack>

namespace imajuscule
{
    class Command;
    class Undoable;
    typedef std::vector<Undoable*> Undoables;
    class Undoable
    {
    public:
        virtual ~Undoable();
        // return false if Command has no effect
        virtual bool Execute() = 0;
        virtual bool Undo() = 0;
        virtual bool Redo() = 0;
        virtual bool Undo(Undoable * limit, bool bStrict, bool & bFoundLimit) = 0;
        virtual bool Redo(Undoable * limit, bool bStrict, bool & bFoundLimit) = 0;
        
        
        void Add(Undoable*); // adds an undoable in the list of sub element or to the current subelement if it exists (see StartSubElement)
        
        void StartSubElement();
        void EndSubElement();
        
        void traverseForward(Undoables::iterator & begin, Undoables::iterator& end) const;

        void traverseForwardRecurse(Undoables &) const;
        
        bool contains(Undoable * u);
        
            
        bool isObsolete() const;

    protected:
        Undoable();

        void setIsObsolete();

        mutable Undoables m_undoables; // mutable because isObsolete() can delete commands

    private:
        std::stack<Undoable*> m_curSubElts;
        bool m_obsolete;
    };
}
