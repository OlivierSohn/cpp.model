
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
        enum { Is = sizeof(Test(static_cast<D*>(nullptr))) == sizeof(Yes) ? 1 : 0 };
    };

    constexpr unsigned int ceil_power_of_two(unsigned int v)
    {
        v = v - 1;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }
}
