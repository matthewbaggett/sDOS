#ifndef sdos_cpp
#define sdos_cpp
#include "includes.h"
#include "debugger.h"
#include "filesystem.h"
#include "wifi.h"

class sDOS {
    public:
        sDOS(){};
        void Run();
        void Loop();
        
    private:
        String _component = "Kernel";
        void _configure();
        Preferences _preferences;
        Debugger _debugger = Debugger();
        FileSystem _fileSystem = FileSystem(_debugger);
        WiFiManager _wifi = WiFiManager(_debugger, _fileSystem);
        long _lastCycleTimeMS = 0;
        long _lastTimeStampUS = 0;

};

void sDOS::Run() {
    _configure();
}

void sDOS::_configure() {
    _debugger.Debug(_component, String("Started Kernel"));
    _wifi.connect();
};

void sDOS::Loop(){
    long microseconds = micros();
    _lastCycleTimeMS = (microseconds - _lastTimeStampUS) / 1000;
    _lastTimeStampUS = microseconds;
    int sleep = random(500,2000);
    //_debugger.Debug(_component, String("Looped in %dms. Sleep %dms"), _lastCycleTimeMS, sleep);
    //delay(sleep);
    _wifi.loop();
}

#endif