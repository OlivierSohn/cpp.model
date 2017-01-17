
namespace imajuscule {
    
    // http://www.gotw.ca/gotw/071.htm
    template<class D, class B>
    class IsDerivedFrom
    {
    private:
        class Yes { char a[1]; };
        class No { char a[10]; };
        
        static Yes Test( B* ); // undefined
        static No Test( ... ); // undefined
        
    public:
        static constexpr bool Is = sizeof(Test(safe_cast<D*>(nullptr))) == sizeof(Yes) ? 1 : 0;
    };
}
