#pragma once

#include "gl.platform.h"

#include "globals.h"

#include "referentiables.h"

#include "referentiable.root.h"

#include "os.abstraction.h"

#include "keyboard.sensor.h"

#include "history.manager.h"

using namespace imajuscule;
namespace {
    class Fixture: public ::testing::Test {
    public:
        void SetUp() {
            try {
                // App initialization //
                LG(INFO, "SetUp initialize managers");
                Referentiables::getManagers();
                ////////////////////////
                
                LG(INFO, "SetUp setGlFPP");
                setGlFPP(true);
                
                LG(INFO, "SetUp registerAbstraction");
                OSAbstraction::registerAbstraction(&os_abstraction);
                LG(INFO, "SetUp Register");
                if(auto s = Sensor::ArrowKeysAlgo::getInstance()) {
                    s->Register();
                }
                LG(INFO, "SetUp SetCurrentGUITime");
                SetCurrentGUITime(0.f);
            }
            catch(...) {
                LG(ERR, "!!! SetUp exception");
                throw;
            }
        }
        
        void TearDown() {
            try {
                LG(INFO, "TearDown Unregister");
                if(auto s = Sensor::ArrowKeysAlgo::getInstance()) {
                    s->Unregister();
                }
                LG(INFO, "TearDown unregisterAbstraction");
                OSAbstraction::unregisterAbstraction(&os_abstraction);
                
                // App termination //
                Globals::reset();
                /////////////////////
            }
            catch(...) {
                LG(ERR, "!!! TearDown exception");
                throw;
            }
        }
        
    private:
        TestOSAbstraction os_abstraction;
    };
}
