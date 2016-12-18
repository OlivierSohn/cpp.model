
namespace imajuscule
{
    template<class T>
    struct ref_unique_ptr : public std::unique_ptr<T, std::function<void(T*)>> {
        ref_unique_ptr(T * ref = 0) :
        std::unique_ptr<T, std::function<void(T*)>>(ref, [](T*r) {
            r->deinstantiate();
        })
        {}
    };
    
    template<class T, class... Args>
    inline ref_unique_ptr<T>
    make_unique_ref(Args&&... args) {
        return ref_unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
