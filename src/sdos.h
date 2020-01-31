#ifndef sdos_cpp
#define sdos_cpp
#include "includes.h"

#include "debugger.h"
#include "events.h"
#include "filesystem.h"
#include "wifi.h"


#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

class sDOS {
    public:
        sDOS(){};
        void Run();
        void Loop();
        
    private:
        String _component = "Kernel";
        void _configure();
        uint32_t _cpuFrequencyUpdate();
        Preferences _preferences;
        Debugger _debugger = Debugger();
        FileSystem _fileSystem = FileSystem(_debugger);
        EventsManager _events = EventsManager(_debugger);
        WiFiManager _wifi = WiFiManager(_debugger, _fileSystem, _events);
        long _lastCycleTimeMS = 0;
        long _lastTimeStampUS = 0;
        uint64_t _loopCount = 0;

};

void sDOS::Run() {
    _configure();
}

uint32_t sDOS::_cpuFrequencyUpdate() {
    #if defined(CPU_FREQ_MHZ)
        uint32_t targetFreq = CPU_FREQ_MHZ_NORADIO;
        if(_wifi.isActive()){
            targetFreq = CPU_FREQ_MHZ;
        }

        if(getCpuFrequencyMhz() != targetFreq){
            setCpuFrequencyMhz(targetFreq);
            _events.trigger("cpu_freq_mhz", getCpuFrequencyMhz());
        }
        
    #endif
    uint32_t currentFreq = getCpuFrequencyMhz();
    if(currentFreq <= 20){
        TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
        TIMERG0.wdt_feed = 1;
        TIMERG0.wdt_wprotect = 0;
    }
    return currentFreq;
}
void sDOS::_configure() {
    _debugger.Debug(_component, String("Started Kernel"));   
};

void sDOS::Loop(){
    // Calculate how long it takes to iterate a loop.
    long microseconds = micros();
    _lastCycleTimeMS = (microseconds - _lastTimeStampUS) / 1000;
    _lastTimeStampUS = microseconds;
    _loopCount++;

    // Check CPU frequency is correct.
    uint32_t cpuFreq = _cpuFrequencyUpdate();
    yield();
    if(_loopCount % 1000 == 0){
        _debugger.Debug(_component, String("Loop executing in %dms at %d Mhz."), _lastCycleTimeMS, cpuFreq);
    }

    // if the wifi manager is active, check the wifi loop
    if(_wifi.isActive()){
        _wifi.loop();
        yield();
    }

    // Check the Events loop
    _events.loop();
    yield();
    
    delay(30);
}

#endif