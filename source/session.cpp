#include "session.h"

using namespace imajuscule;
Session * Session::g_Instance = NULL;

Session * Session::getInstance()
{
    if (!g_Instance)
        g_Instance = new Session();

    return g_Instance;
}

Session::~Session()
{}
