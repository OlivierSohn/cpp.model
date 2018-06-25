

namespace imajuscule {
    struct RAII
    {
        using F = std::function<void(void)>;
        
        F fDestructor;
        RAII(F const & fConstructor, F const & fDestructor):
        fDestructor(fDestructor)
        {
            fConstructor();
        }
        ~RAII()
        {
            fDestructor();
        }
    };

    struct inc_dec_RAII : public RAII
    {
        static constexpr auto minVal = 0;
        inc_dec_RAII(int & i) : RAII([&i]() {
            Assert(i >= minVal);
            i++;
        }, [&i]() {
            --i;
            Assert(i >= minVal);
        }) {}
    };

}
