
namespace imajuscule {
    struct Application {
        static void Init() {
            //LG(INFO, "Application::Init ...");
            Referentiables::getManagers();
            //LG(INFO, "Application::Init Done");
        }
        
        static void TearDown() {
            //LG(INFO, "Application::TearDown ...");
            Globals::reset();
            //LG(INFO, "Application::TearDown Done");
        }
    };
    
    struct ScopedApplication {
        ScopedApplication() {
            Application::Init();
        }
        ~ScopedApplication() {
            Application::TearDown();
        }
    };
}
