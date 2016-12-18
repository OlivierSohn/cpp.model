
namespace imajuscule
{
    class ReferentiableRoot : public Referentiable
    {
        friend class Globals;
        DEFINE_REF_WITH_VISITOR(ReferentiableRoot);
    public:
        static ReferentiableRoot * getInstance();

        void initialize();
    protected:
        ReferentiableRoot(ReferentiableManagerBase * manager, const std::string & guid, const std::string & hintName);

        DECL_PERSIST(ReferentiableRoot, Referentiable)
        
        static intrusive_ptr<ReferentiableRoot> g_instance;
    };
}

