
namespace imajuscule
{
    class ReferentiableManagerBase;
    
    typedef std::vector<ReferentiableManagerBase*> managers;
    class Referentiables final
    {
        friend class Globals;
    public:
        static Referentiable* fromGUID(const DirectoryPath & path, const std::string &);
        static Referentiable* fromGUIDLoaded(const std::string &);
        static Referentiable* fromSessionNameLoaded(const std::string &);
        static managers const & getManagers();
        
        void regManager(ReferentiableManagerBase *);

        template<typename F>
        static void forEach(F f) {
            auto r = getInstance();
            if(!r) {
                return;
            }
            for(auto * m : r->getManagers()) {
                if(m) {
                    m->forEach(f);
                }
            }
        }
    private:
        Referentiables();
        static Referentiables * getInstance();
        static Referentiables* m_instance;
        managers m_managers;

        Referentiable* findRefFromGUID(const DirectoryPath & path, const std::string &);
        Referentiable* findRefFromGUIDLoaded(const std::string &);
        Referentiable* findRefFromSessionNameLoaded(const std::string &);
    };
}
