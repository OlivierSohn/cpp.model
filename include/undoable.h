#pragma once

#include <vector>
#include <stack>
#include <string>

namespace imajuscule
{
    class Command;
    class Undoable;
    typedef std::vector<Undoable*> Undoables;
    class Undoable
    {
    public:
        
        enum ExecType
        {
            UNDO,
            REDO,
            NONE
        };

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

        void getExtendedDescription(std::string & desc, size_t offset);
        virtual void getDescription(std::string & desc) = 0;

    protected:
        Undoable();

        void setIsObsolete();

        mutable Undoables m_undoables; // mutable because isObsolete() can delete commands

    private:
        std::stack<Undoable*> m_curSubElts;
        bool m_obsolete;
    };
}

#include "os.log.h"
// depending on where this MACRO is used, CommandResult will be XXX::CommandResult or YYY::CommandResult

#define RESULT_BY_REF(r) \
[&](const Command::CommandResult * res){                                \
if_A(res)                                                                 \
{                                                                           \
if_A(res->initialized());                                                 \
{                                                                          \
const CommandResult* myRes = dynamic_cast<const CommandResult*>(res);   \
if_A(myRes)                                                             \
{                                                                       \
r = *myRes;                                                        \
A(r.initialized());                                                \
}                                                                     \
}                                                                       \
}                                                                          \
}

#define SUBCR_LISTEN_TO_RESULT \
static FunctionInfo<Event> ListenToResult(Command & c, CommandResult & r)       \
{                                                                               \
return c.observable().Register(Event::RESULT, RESULT_BY_REF(r));            \
}

#define SUBCR_DFLT_CONTRUCTOR \
CommandResult() : Command::CommandResult() {}

#define SUBCR_DESTRUCTOR  \
~CommandResult() {}

#define SUBCR \
public:                           \
SUBCR_DFLT_CONTRUCTOR;        \
SUBCR_DESTRUCTOR;             \
SUBCR_LISTEN_TO_RESULT;       \
private:


