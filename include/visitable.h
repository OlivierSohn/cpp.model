
#define VISITOR_HEADER_IMPL void accept(Visitor & vtor) override {\
    vtor.Visit(this);\
}


namespace imajuscule
{
    struct Object {
        virtual ~Object() = default;
        Object() = default;
    };

    struct NonCopyable : public Object {
        NonCopyable() =default;
        NonCopyable(const NonCopyable &) = delete;
        NonCopyable & operator=(const NonCopyable&) = delete;
    };
    
    class Visitor;
    class Visitable : public Object
    {
    public:
        enum class HierarchyEvent : unsigned char{
            ADD_CHILD,
            REMOVE_CHILD,
            
            SIZE_ENUM
        };
        enum class Event: unsigned char {
            VISITABLE_DELETE,
            
            SIZE_ENUM // we use the one in other enum
        };
        
        ~Visitable();

        Observable<Event, Visitable&> &  observableVisitable();
        Observable<HierarchyEvent, Visitable&, Visitable&> &  observableVisitableH();

        virtual void accept(Visitor &) = 0;
    protected:
        Visitable();

    private:
        Observable<Event, Visitable&> * m_observableVisitable;
        Observable<HierarchyEvent, Visitable&, Visitable&> * m_observableVisitableH;
        
        Visitable(const Visitable &) = delete;
        Visitable & operator=(const Visitable&) = delete;
    };
}
