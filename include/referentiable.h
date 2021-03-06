
#define DEFINE_REF(x) friend class ReferentiableManager<x>

#define DEFINE_REF_WITH_VISITOR(x) public:  \
VISITOR_HEADER_IMPL \
private: \
friend class ReferentiableManager<x>
// last line without a ';'

namespace imajuscule
{
    class ReferentiableManagerBase;
    
    template <typename>
    class ReferentiableManager; // so that all referentiables can friend their manager

    class ReferentiableCmdBase;
    class Command;
    class Referentiable;
    typedef std::vector<Referentiable*> refs;

    class Referentiable : public Persistable
    {
        friend class ReferentiableManagerBase;
    public:
        Referentiable(Referentiable&& other)
        {
            m_guid = std::move(other.m_guid);
            m_hintName = std::move(other.m_hintName);
            m_sessionName = std::move(other.m_sessionName);
            m_dateOfCreation = std::move(other.m_dateOfCreation);
            m_observableReferentiable = std::move(other.m_observableReferentiable);
        }
        
        Referentiable& operator=(Referentiable&& other)
        {
            if (this != &other)
            {
                m_guid = std::move(other.m_guid);
                m_hintName = std::move(other.m_hintName);
                m_sessionName = std::move(other.m_sessionName);
                m_dateOfCreation = std::move(other.m_dateOfCreation);
                m_observableReferentiable = std::move(other.m_observableReferentiable);
            }
            return *this;
        }
        
        void deinstantiate();
        enum class Event: unsigned char
        {
            WILL_BE_DELETED,
            
            SIZE_ENUM
        };
        Observable<Event, Referentiable*> * observableReferentiable();

        const std::string & guid() const;
        const std::string & sessionName() const;
        std::string extendedName() const;
        const std::string & hintName() const;
        const std::string & creationDate() const;

        ReferentiableManagerBase * getManager() const { return m_manager; }

        void Hide();
        bool isHidden();
        
        static bool ReadIndexForDiskGUID(const DirectoryPath & path, const std::string & guid, unsigned int &index, std::string & sHintName);
        
        int16_t get_shared_counter() const { return shared_count; }
        int16_t & edit_shared_counter() { return shared_count; }
        
    protected:
        
        virtual ~Referentiable() = default;

        virtual void Init() {};

        // this version of the contructor doesn't set creation Date 
        Referentiable();
        Referentiable(ReferentiableManagerBase * manager, std::string && guid);
        Referentiable(ReferentiableManagerBase * manager, std::string && guid, const std::string & hintName);
        
        DECL_PERSIST(Referentiable, Persistable)
        
        class ReferentiableIndexLoad : public KeysLoad
        {
        public:
            ReferentiableIndexLoad( DirectoryPath, FileName);
            
            bool found(unsigned int &index, std::string & nameHint);
            const std::string & dateCrea();
        
        protected:
            void LoadInt32ForKey(char key, int32_t i) override;
            void LoadStringForKey(char key, std::string & sVal) override;

        private:
            bool m_bFound : 1;
            std::string m_hintName, m_dateOfCreation;
            unsigned int m_uiIndex;
        };
        
    private:
        int16_t shared_count = intrusive_ptr_zero_count;
        ReferentiableManagerBase * m_manager;
        std::string m_guid; // persisted
        std::string m_hintName; // persisted
        std::string m_sessionName; // not persisted
        std::string m_dateOfCreation; // persisted
        Observable<Event, Referentiable*> * m_observableReferentiable;

        virtual void setSessionName(const std::string & sn);

        virtual Referentiable * mainRefAttr() const;
        void deleteObservableReferentiable();
    };
    
    template<class T, typename std::enable_if<IsDerivedFrom<T, Referentiable>::value>::type* = nullptr >
    using ref_weak_ptr = WeakPtrBase< T, Referentiable, &Referentiable::observableReferentiable, Referentiable::Event::WILL_BE_DELETED>;
}
#define SET_ref_unique(type, name, methodPostFix) \
void set##methodPostFix(type * p) { \
    if(p == name) {\
        return;\
    }\
    removeSpec(name.get());\
    name.reset(p);\
    addSpec(name.get());\
}


