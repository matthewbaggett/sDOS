#ifndef sdos_cpp
#define sdos_cpp
#include "includes.h"

#include "debugger.h"
#include "events.h"
#include "filesystem.h"
#include "wifi.h"

#include "abstracts/abstract_accellerometer.h"
#include "abstracts/abstract_rtc.h"

#include "abstracts/abstract_sdos_driver.h"
#include "abstracts/abstract_sdos_service.h"

#ifdef ENABLE_POWER
#include "drivers/power.h"
#endif
#ifdef ENABLE_I2C
#include "drivers/i2c.h"
#endif
#ifdef ENABLE_TTP223
#include "drivers/touch/ttp223.h"
#endif
#ifdef ENABLE_PCF8563
#include "drivers/rtc/pcf8563.h"
#endif
#ifdef ENABLE_MPU9250
#include "drivers/accellerometer/mpu9250.h"
#endif

// System Services
#ifdef ENABLE_SERVICE_NTP
#include "services/ntp.h"
#endif
#ifdef ENABLE_SERVICE_SLEEPTUNE
#include "services/sleeptune.h"
#endif

using namespace std;

class sDOS
{
public:
    sDOS();
    void Setup();
    void Loop();

private:
    String _component = "Kernel";
    std::map<std::string, sDOS_Abstract_Driver *> _drivers;
    std::map<std::string, sDOS_Abstract_Service *> _services;
    void _configure();
    uint32_t _cpuFrequencyUpdate();
    Preferences _preferences;
    Debugger _debugger = Debugger();
    FileSystem _fileSystem = FileSystem(_debugger);
    EventsManager _events = EventsManager(_debugger);
    WiFiManager * driver_WiFi = new WiFiManager(_debugger, _fileSystem, _events);
    long _lastCycleTimeMS = 0;
    long _lastTimeStampUS = 0;
    uint64_t _loopCount = 0;
    static uint driverId;
    std::string getDriverID(char* name);
};

sDOS::sDOS(){

};

uint sDOS::driverId = 0;

std::string sDOS::getDriverID(char* name){
    char id[12];

    Serial.printf("name:  %s\n", name);
    Serial.printf("driverId:  %d\n", driverId);
    sprintf(id, "%d-%.8s", driverId, name);
    Serial.printf("driver index: %s\n", id);
    driverId++;

    return id;
}

void sDOS::Setup()
{
    Serial.begin(SERIAL_BAUD);
    Serial.setDebugOutput(SERIAL_DEBUG_ENABLE);
    delay(300);
    _debugger.Debug(_component, F("Started Smol Device Operating System Kernel"));
    _debugger.Debug(_component, F("Built with love on %s at %s."), __DATE__, __TIME__);
    
    _drivers.insert(std::make_pair(getDriverID("wifi"), driver_WiFi));
    _cpuFrequencyUpdate();
#ifdef ENABLE_POWER
    _drivers.insert(std::make_pair(getDriverID("power"), new SDOS_POWER(_debugger, _events)));
#endif
#ifdef ENABLE_I2C
    SDOS_I2C * driver_I2C = new SDOS_I2C(_debugger, _events);
    _drivers.insert(std::make_pair(getDriverID("i2c"), driver_I2C));
#endif
#ifdef ENABLE_TTP223
    _drivers.insert(std::make_pair(getDriverID("touch"), new SDOS_TTP223(_events)));
#endif
#ifdef ENABLE_PCF8563
    SDOS_PCF8563 * driver_RTC = new SDOS_PCF8563(_events, driver_I2C);
    _drivers.insert(std::make_pair(getDriverID("rtc"), driver_RTC));
#endif
#ifdef ENABLE_MPU9250
    _drivers.insert(std::make_pair(getDriverID("accellerometer"), new SDOS_MPU9250(_events)));
#endif
#ifdef ENABLE_SERVICE_NTP
    _services.insert(std::make_pair(getDriverID("ntp"), new SDOS_NTP(_debugger, _events, driver_RTC, driver_WiFi)));
#endif
#ifdef ENABLE_SERVICE_SLEEPTUNE
    _services.insert(std::make_pair(getDriverID("sleeptune"), new SDOS_SLEEPTUNE(_debugger, _events, driver_WiFi)));
#endif

    for (std::map <std::string, sDOS_Abstract_Driver *>::iterator it = _drivers.begin(); it != _drivers.end(); ++it)
    {
        std::string name = it->first;
        sDOS_Abstract_Driver *driver = it->second;
        //_debugger.Debug(_component, "Driver::Setup::%s", name.c_str());
        driver->setup();
    }

    for (std::map<std::string, sDOS_Abstract_Service *>::iterator it = _services.begin(); it != _services.end(); ++it)
    {
        std::string name = it->first;
        sDOS_Abstract_Service *service = it->second;
        //_debugger.Debug(_component, "Service::Setup::%s", name.c_str());
        service->setup();
    }
};

uint32_t sDOS::_cpuFrequencyUpdate()
{
#ifdef CPU_FREQ_MHZ
    uint32_t targetFreq = CPU_FREQ_MHZ;
    /*
    _debugger.Debug(
        _component,
        "_wifi.isActive = %s, _wifi.getRequestCount = %d, WiFi.isConnected = %s",
        _wifi.isActive() ? "yes" : "no",
        _wifi.getRequestCount(), 
        WiFi.isConnected() ? "yes" : "no"
    );
    */
#ifdef CPU_FREQ_MHZ_NORADIO
    if (driver_WiFi->canSleep())
    {
        targetFreq = CPU_FREQ_MHZ_NORADIO;
    }
#endif
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
    long _lastCycleTimeUS = microseconds - _lastTimeStampUS;
    _lastCycleTimeMS = _lastCycleTimeUS / 1000;

    _lastTimeStampUS = microseconds;
    _loopCount++;

    // Check CPU frequency is correct.
    uint32_t cpuFreq = _cpuFrequencyUpdate();
    yield();
    if (_loopCount % 25000 == 0)
    {
        _debugger.Debug(_component, F("Loops are executing in %dms at %d Mhz."), _lastCycleTimeMS, cpuFreq);
    }

    // if the wifi manager is active, check the wifi loop
    if (driver_WiFi->isActive())
    {
        driver_WiFi->loop();
        yield();
    }

    // Check the Events loop
    _events.loop();
    yield();

    // Loop over Drivers
    for (std::map<std::string, sDOS_Abstract_Driver *>::iterator it = _drivers.begin(); it != _drivers.end(); ++it)
    {
        std::string name = it->first;
        sDOS_Abstract_Driver *driver = it->second;
        //_debugger.Debug(_component, "Driver::Loop::%s", name.c_str());
        if(driver->isActive()){
            driver->loop();
            yield();
        }
    }

    // Loop over Services
    for (std::map<std::string, sDOS_Abstract_Service *>::iterator it = _services.begin(); it != _services.end(); ++it)
    {
        std::string name = it->first;
        sDOS_Abstract_Service *service = it->second;
        //_debugger.Debug(_component, "Service::Loop::%s",  name.c_str());
        service->loop();
        yield();
    }
}

#endif