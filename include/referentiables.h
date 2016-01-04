#pragma once

#include <vector>

#include "referentiable.h"

namespace imajuscule
{
    class ReferentiableManagerBase;
    
    typedef std::vector<ReferentiableManagerBase*> managers;
    class Referentiables
    {
    public:
        static Referentiable* fromGUID(const DirectoryPath & path, const std::string &);
        static Referentiable* fromGUIDLoaded(const std::string &);
        static Referentiable* fromSessionNameLoaded(const std::string &);
        static void registerManager(ReferentiableManagerBase &);
        static void traverseManagers(managers::iterator & begin, managers::iterator & end);

    private:
        Referentiables();
        virtual ~Referentiables();
        static Referentiables * getInstance();
        static Referentiables * m_instance;
        managers m_managers;

        Referentiable* findRefFromGUID(const DirectoryPath & path, const std::string &);
        Referentiable* findRefFromGUIDLoaded(const std::string &);
        Referentiable* findRefFromSessionNameLoaded(const std::string &);
        void regManager(ReferentiableManagerBase &);
    };
}
