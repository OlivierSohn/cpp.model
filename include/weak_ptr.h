
namespace imajuscule
{
    template<
    class T,
    class U,
    Observable<typename U::Event, U*>* (U::*fObservable)(),
    typename U::Event DeleteEvent
    >
    struct WeakPtrBase : public NonCopyable {
        using Event = typename U::Event;

        WeakPtrBase(WeakPtrBase && o) {
            o.Unregister();
            set(o.ptr);
        }
        
        WeakPtrBase & operator = (WeakPtrBase && o) {
            o.Unregister();
            set(o.ptr);
            return *this;
        }
        
        WeakPtrBase() = default;
        
        WeakPtrBase(T*p) {
            set(p);
        }
        
        ~WeakPtrBase() {
            reset();
        }
        
        WeakPtrBase & operator=(T*p) {
            set(p);
            return *this;
        }
        
        operator T*() const { return ptr; }
        operator bool() const { return !!ptr; }
        T& operator*() const { return *ptr; }
        T* operator -> () const { return ptr; }
        
        bool operator == (T*o) const {
            return ptr == o;
        }
        bool operator != (T*o) const {
            return ptr != o;
        }
        
        bool operator == (WeakPtrBase const & o) const {
            return ptr == o.ptr;
        }
        bool operator != (WeakPtrBase const & o) const {
            return ptr != o.ptr;
        }
        
        void set(T*b) {
            if( b == ptr) {
                return;
            }

            reset();
            ptr = b;
            Register();
        }
        
        T * get() const { return ptr; }
        
        void reset() {
            Unregister();
            ptr = nullptr;
        }
        
    private:
        T * ptr = 0;
        std::vector<FunctionInfo<Event>> m_reg;

        void Register() {
            if(!ptr) {
                return;
            }
            
            if(auto o = std::bind(fObservable, ptr)() ) {
                m_reg.emplace_back(o->Register(DeleteEvent, [this](U*){
                    ptr = nullptr;
                    m_reg.clear();
                }));
            } else {
                // ptr is already deleted
                ptr = 0;
            }
        }
        
        void Unregister() {
            if(ptr) {
                if(auto o = std::bind(fObservable, ptr)() ) {
                    o->Remove(m_reg);
                }
            }
            m_reg.clear();
        }
    };    
}
