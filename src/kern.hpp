#ifndef sdos_cpp
#define sdos_cpp
#include "kern_inc.h"

#include "debugger.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "wifi.hpp"
#include "bluetooth.hpp"

#ifdef ENABLE_POWER
#include "drivers/power.hpp"
#endif
#ifdef ENABLE_I2C
#include "drivers/i2c.hpp"
#endif
#ifdef ENABLE_TTP223
#include "drivers/touch/ttp223.hpp"
#endif
#ifdef ENABLE_PCF8563
#include "drivers/rtc/pcf8563.hpp"
#endif
#ifdef ENABLE_MPU9250
#include "drivers/accellerometer/mpu9250.hpp"
#endif

// System Services
#ifdef ENABLE_SERVICE_NTP
#include "services/ntp.hpp"
#endif
#ifdef ENABLE_SERVICE_SLEEPTUNE
#include "services/sleeptune.hpp"
#endif

using namespace std;
using driverMap = std::map<String, sDOS_Abstract_Driver *>;
using serviceMap = std::map<String, sDOS_Abstract_Service *>;

class sDOS
{
public:
    sDOS();
    void Setup();
    void Loop();

private:
    String _component = "Kernel";
    driverMap _drivers;
    serviceMap _services;
    void _configure();
    uint32_t _cpuFrequencyUpdate();
    Preferences _preferences;
    Debugger _debugger = Debugger();
    FileSystem _fileSystem = FileSystem(_debugger);
    EventsManager _events = EventsManager(_debugger);
    WiFiManager * driver_WiFi = new WiFiManager(_debugger, _fileSystem, _events);
    BluetoothManager * driver_BT = new BluetoothManager(_debugger, _events);
    long _lastCycleTimeMS = 0;
    long _lastTimeStampUS = 0;
    uint64_t _loopCount = 0;
    static uint driverId;
    String getDriverID(String name);
};

sDOS::sDOS(){

};

uint sDOS::driverId = 0;

String sDOS::getDriverID(String name)
{
    char id[12];
    sprintf(id, "%d-%.8s", driverId, name.c_str());
    driverId++;
    return String(id);
}

void sDOS::Setup()
{
    Serial.begin(SERIAL_BAUD);
    Serial.setDebugOutput(SERIAL_DEBUG_ENABLE);
    delay(300);

#ifdef ENABLE_POWER
    _drivers.insert(std::make_pair(getDriverID(F("power")), new SDOS_POWER(_debugger, _events)));
#endif

    _debugger.Debug(_component, F("Started Smol Device Operating System Kernel"));
    _debugger.Debug(_component, F("Built with love on %s at %s."), __DATE__, __TIME__);

    _drivers.insert(std::make_pair(getDriverID(F("wifi")), driver_WiFi));
    _drivers.insert(std::make_pair(getDriverID(F("Bluetooth")), driver_BT));
    _cpuFrequencyUpdate();

#ifdef ENABLE_I2C
    SDOS_I2C * driver_I2C = new SDOS_I2C(_debugger, _events);
    _drivers.insert(std::make_pair(getDriverID(F("i2c")), driver_I2C));
#endif
#ifdef ENABLE_TTP223
    _drivers.insert(std::make_pair(getDriverID(F("touch")), new SDOS_TTP223(_events)));
#endif
#ifdef ENABLE_PCF8563
    SDOS_PCF8563 * driver_RTC = new SDOS_PCF8563(_debugger, _events, driver_I2C);
    _drivers.insert(std::make_pair(getDriverID(F("rtc")), driver_RTC));
#endif
#ifdef ENABLE_MPU9250
    _drivers.insert(std::make_pair(getDriverID(F("accellerometer")), new SDOS_MPU9250(_events)));
#endif

#ifdef ENABLE_SERVICE_SLEEPTUNE
    _services.insert(std::make_pair(getDriverID(F("sleeptune")), new SDOS_SLEEPTUNE(_debugger, _events, driver_WiFi, driver_BT)));
#endif
#ifdef ENABLE_SERVICE_NTP
    _services.insert(std::make_pair(getDriverID(F("ntp")), new SDOS_NTP(_debugger, _events, driver_RTC, driver_WiFi)));
#endif

    for (driverMap::iterator it = _drivers.begin(); it != _drivers.end(); ++it)
    {
        String name = it->first;
        sDOS_Abstract_Driver *driver = it->second;
        //_debugger.Debug(_component, "Driver::Setup::%s", name.c_str());
        driver->setup();
    }

    for (serviceMap::iterator it = _services.begin(); it != _services.end(); ++it)
    {
        String name = it->first;
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
        "_wifi.isActive = %s, _wifi.getRequestCount = %d, WiFi.isConnected = %s, cansleep? %s",
        driver_WiFi->isActive() ? "yes" : "no",
        driver_WiFi->getRequestCount(), 
        WiFi.isConnected() ? "yes" : "no",
        driver_WiFi->canSleep() ? "yes" : "no"
    );
    */
#ifdef CPU_FREQ_MHZ_NORADIO
    if (driver_WiFi->canSleep() && driver_BT->canSleep())
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
    if (_loopCount % SLEEPTUNE_LOOPS_PER_TIMING_REPORT == 0)
    {
        //_debugger.Debug(_component, F("Loops (%d) are executing in %dms at %d Mhz."), SLEEPTUNE_LOOPS_PER_TIMING_REPORT, _lastCycleTimeMS, cpuFreq);
    }

    // Loop over Drivers
    for (driverMap::iterator it = _drivers.begin(); it != _drivers.end(); ++it)
    {
        String name = it->first;
        sDOS_Abstract_Driver *driver = it->second;
        //_debugger.Debug(_component, "Driver::Loop::%s (active=%s)", name.c_str(), driver->isActive() ? "yes" : "no");
        if(driver->isActive()){
            driver->loop();
            yield();
        }
    }

    // Loop over Services
    for (serviceMap::iterator it = _services.begin(); it != _services.end(); ++it)
    {
        String name = it->first;
        sDOS_Abstract_Service *service = it->second;
        //_debugger.Debug(_component, "Service::Loop::%s (active=%s)", name.c_str(), service->isActive() ? "yes" : "no");
        if(service->isActive()){
            service->loop();
            yield();
        }
    }

    // Check the Events loop
    _events.loop();
    yield();
}

void Debugger::Debug(String component, String format, ...)
{
    char buff[128];
    va_list args;
    va_start(args, format.c_str());
    vsprintf(buff, format.c_str(), args);

    if (Debugger::lastComponent.equals(component) && strcmp(buff, Debugger::lastBuff) == 0)
    {
        Debugger::duplicates++;
        _serial.printf("\t\t(repeated %d times)\r", Debugger::duplicates);
        return;
    }

    component.toUpperCase();

    char outputBuffer[sizeof(buff)];
    snprintf(
        outputBuffer, 
        sizeof(outputBuffer), 
        "%s[%s%.7-7s%s %s%dMhz%s %s%s%s %s%s%s %s%dmV%s] %s\n", 
        Debugger::duplicates > 0 ? "\n":"", 
            COL_YELLOW,
            component.c_str(), 
            COL_RESET,
            getCpuFrequencyMhz() > 20 ? COL_RED : COL_GREEN,
            getCpuFrequencyMhz(),
            COL_RESET,
            WiFi.isConnected() ? COL_RED : COL_GREEN,
            WiFi.isConnected() ? "W+" : "W-",
            COL_RESET, 
            sdos_is_bluetooth_active() ? COL_RED : COL_GREEN,
            sdos_is_bluetooth_active() ? "B+" : "B-",
            COL_RESET,
            SDOS_POWER::isCharging() ? COL_BLUE : COL_PINK,
            SDOS_POWER::getVbattMv(),
            COL_RESET,
            buff
    );

    _serial.print(outputBuffer);

    for (auto const& it : Debugger::_handlers) {
        it(outputBuffer);
    }

    memcpy(Debugger::lastBuff, buff, sizeof(buff));
    Debugger::lastComponent = component;
    Debugger::duplicates = 0;
    return;
};

#endif