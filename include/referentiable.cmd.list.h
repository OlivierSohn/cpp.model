#pragma once

#include "command.h"

namespace imajuscule
{
#define REF_CMD_LIST template \
        < class T /* Type of modified object (is a referentiable)*/ \
        , class U /*Type of object attribute (is a Referentiable)*/ \
        , void (T::*fAdd)(U*) \
        , bool (T::*fRemove)(U*) >

    class ReferentiableManagerBase;
    
    REF_CMD_LIST
    class RefAttrListCmd : public Command
    {
        friend T;
    public:
        void getSentenceDescription(std::string & desc) const override;

        enum class Type
        {
            TYPE_ADD,
            TYPE_REMOVE
        };
        static Type Other(Type);
        static bool ManageAttr(T & obj, U * newAttr, Type t, bool & found);

    protected:
        RefAttrListCmd(T & ref, const U * attr, Type t);

        class CommandResult : public Command::CommandResult
        {
            SUBCR
            bool found;
        public:
            CommandResult(bool bSuccess, bool found);
            bool getFound() const { return found; }
        };
    private:
        struct data : public Command::data
        {
            Type m_t;
            std::string m_attrGUID;
            ReferentiableManagerBase * m_manager;

            bool operator!=(const Command::data&) const override;
            std::string getDesc() const override;

            data(const U*, Type t);
            static Command::data * instantiate(const U * attr, Type t);
            U * Attr() const;
        };

        const char * m_shortDesc;
        Type m_type;

        bool doExecute(const Command::data & Data) override;

        // returns true if a command was executed
        static bool ExecuteFromInnerCommand(T & obj, U * newAttr, Type t, bool & bSuccess /*returns true if executed command succeeded*/, bool & found);

        // returns true if command succeeded
        static bool Execute(T & obj, const U * attr, Type t, bool & found);
    };
}
