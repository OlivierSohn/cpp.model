#pragma once

#include <string>

// the session is responsible for loading persistables, saving persitables using PersistableManager(s) Objects
namespace imajuscule
{
    class Session
    {
    public:
        static Session * getInstance();

    private:
        Session(); // a session can't be subclassed
        virtual ~Session();
        static Session * g_Instance;
    };
}
