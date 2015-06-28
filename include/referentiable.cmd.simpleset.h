#pragma once

#include "referentiable.h"
#include "command.h"

namespace imajuscule
{
#define REF_CMD_SIMPLESET template \
        < class T /* Type of modified object (is a referentiable)*/ \
        , class U /*Type of object attribute (is a Referentiable)*/ \
        , void (T::*fSet)(U*) \
        , U* (T::*fGet)() > 

    REF_CMD_SIMPLESET
    class RefSimpleChangeAttrCmd : public Command
    {
        friend T;
    public:
        void getSentenceDescription(std::string & desc) const override;

        static void ChangeAttr(T & obj, U * newAttr);
            
    protected:
        RefSimpleChangeAttrCmd(T & ref, const U * attr);
        ~RefSimpleChangeAttrCmd();

        class CommandResult : public Command::CommandResult
        {
            SUBCR
        public:
            CommandResult(bool bSuccess);
        private:
        };
    private:
        struct data : public Command::data
        {
            bool m_hasAttr;
            std::string m_attrGUID;
            ReferentiableManagerBase * m_manager;

            bool operator!=(const Command::data&) const override;
            std::string getDesc() const override;

            data(const U*);
            data(T&);
            static Command::data * instantiate(const U * attr);
            static Command::data * instantiate(T & obj);
            U * Attr() const;
        };

        const char * m_shortDesc;

        bool doExecute(const Command::data & Data) override;

        // returns true if a command was executed
        static bool ExecuteFromInnerCommand(T & obj, U * newAttr, bool & bSuccess /*returns true if executed command succeeded*/);

        // returns true if command succeeded
        static bool Execute(T & obj, const U * attr);
    };
}
