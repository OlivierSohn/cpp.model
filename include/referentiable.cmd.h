#pragma once

#include "referentiable.h"
#include "command.h"

namespace imajuscule
{
#define R_CMD_TPL_DEF template \
        < class T /* Type of modified object (is a referentiable)*/ \
        , class U /*Type of object attribute (is a Referentiable)*/ \
        , bool (T::*fSet)(U*, bool&) \
        , U* (T::*fGet)() const > 

    R_CMD_TPL_DEF
    class RefChangeAttrCmd : public Command
    {
        friend T;
    public:
        void getSentenceDescription(std::string & desc) override;

        static bool ChangeAttr(T & obj, U * newAttr, bool & bAttrChanged);
            
    protected:
        RefChangeAttrCmd(T & ref, const U * attr);
        ~RefChangeAttrCmd();

        class CommandResult : public Command::CommandResult
        {
            SUBCR;
        public:
            CommandResult(bool bSuccess, bool bAttrChanged);

            bool getAttrChanged() const;
        private:
            bool m_bAttrChanged;
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
        Command * m_preconditionning;

        bool doExecute() override;
        bool doUndo() override;
        bool doRedo() override;

        bool doExecute(const Command::data & Data) override;

        // returns true if a command was executed
        static bool ExecuteFromInnerCommand(T & obj, U * newAttr, bool & bSuccess /*returns true if executed command succeeded*/, bool & bAttrChanged);

        // returns true if command succeeded
        static bool Execute(T & obj, const U * attr, bool & bAttrChanged);
    };
}
