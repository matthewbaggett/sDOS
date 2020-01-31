#ifndef sdos_cpp
#define sdos_cpp
#include "includes.h"

#include "debugger.h"
#include "events.h"
#include "filesystem.h"
#include "wifi.h"

#ifdef ENABLE_I2C
#include "drivers/i2c.h"
#endif
#ifdef ENABLE_TTP223
#include "drivers/touch/ttp223.h"
#endif
#ifdef ENABLE_PCF8563
#include "drivers/rtc/pcf8563.h"
#endif

class sDOS
{
public:
    sDOS();
    void Setup();
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
#ifdef ENABLE_I2C
    SDOS_I2C _i2c = SDOS_I2C(_debugger, _events);
#endif
#ifdef ENABLE_TTP223
    SDOS_TTP223 _ttp223 = SDOS_TTP223(_events);
#endif
#ifdef ENABLE_PCF8563
    SDOS_PCF8563 _pcf8563 = SDOS_PCF8563(_events, _i2c);
#endif
    long _lastCycleTimeMS = 0;
    long _lastTimeStampUS = 0;
    uint64_t _loopCount = 0;
};

sDOS::sDOS(){
   
};

void sDOS::Setup(){
    Serial.begin(SERIAL_BAUD);
    Serial.setDebugOutput(SERIAL_DEBUG_ENABLE);
    delay(300);
    _debugger.Debug(_component, String("Started Smol Device Operating System Kernel"));
    _debugger.Debug(_component, "Built with love on %s at %s.", __DATE__, __TIME__);
    _cpuFrequencyUpdate();
#ifdef ENABLE_I2C
    _i2c.setup();

    _i2c.connect();
    _i2c.scan();
#endif
#ifdef ENABLE_TTP223
    _ttp223.setup();
#endif
#ifdef ENABLE_PCF8563
    _pcf8563.setup();
#endif
};

uint32_t sDOS::_cpuFrequencyUpdate()
{
#if defined(CPU_FREQ_MHZ)
    uint32_t targetFreq = CPU_FREQ_MHZ_NORADIO;
    if (_wifi.isActive())
    {
        targetFreq = CPU_FREQ_MHZ;
    }
    uint32_t currentFreq = getCpuFrequencyMhz();
    if (currentFreq != targetFreq)
    {
        setCpuFrequencyMhz(targetFreq);
        _debugger.Debug("core", "CPU frequency changed from %dMhz to %dMhz", currentFreq, getCpuFrequencyMhz());
        _events.trigger("cpu_freq_mhz", getCpuFrequencyMhz());
    }

#endif
    currentFreq = getCpuFrequencyMhz();
    if (currentFreq <= 20)
    {
        TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
        TIMERG0.wdt_feed = 1;
        TIMERG0.wdt_wprotect = 0;
    }
    return currentFreq;
}

void sDOS::Loop()
{
    // Calculate how long it takes to iterate a loop.
    long microseconds = micros();
    _lastCycleTimeMS = (microseconds - _lastTimeStampUS) / 1000;
    _lastTimeStampUS = microseconds;
    _loopCount++;

    // Check CPU frequency is correct.
    uint32_t cpuFreq = _cpuFrequencyUpdate();
    yield();
    if (_loopCount % 1000 == 0)
    {
        _debugger.Debug(_component, String("Loops are executing in %dms at %d Mhz."), _lastCycleTimeMS, cpuFreq);
    }

    // if the wifi manager is active, check the wifi loop
    if (_wifi.isActive())
    {
        _wifi.loop();
        yield();
    }

    // Check the Events loop
    _events.loop();
    yield();

#ifdef ENABLE_I2C
    _i2c.loop();
#endif
#ifdef ENABLE_TTP223
    _ttp223.loop();
#endif
#ifdef ENABLE_PCF8563
    _pcf8563.loop();
#endif

    delay(30);
}

#endif