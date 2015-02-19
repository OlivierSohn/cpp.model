#pragma once

namespace imajuscule
{
    class Command;
    class Undoable
    {
    public:
        enum State
        {
            NOT_EXECUTED,
            EXECUTED,
            UNDONE,
            REDONE
        };
        static const char * StateToString(State);

        // return false if Command has no effect
        virtual bool Execute() = 0;
        virtual bool Undo() = 0;
        virtual bool Redo() = 0;
        
        virtual void Add(Command*) = 0;

        State getState() const;
        bool isObsolete() const;

        bool validStateToExecute() const;
        bool validStateToUndo() const;
        bool validStateToRedo() const;

    protected:
        Undoable();
        virtual ~Undoable();

        void setState(State state);
        void setIsObsolete();

    private:
        State m_state;
        bool m_obsolete;
    };
}
