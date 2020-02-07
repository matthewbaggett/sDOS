#ifndef sdos_cpp
#define sdos_cpp
#include "kern_inc.h"

unsigned int _loopCount = 0;

#include "debugger.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "wifi.hpp"
#include "bluetooth/bluetooth.hpp"

#ifdef ENABLE_POWER
#include "drivers/power.hpp"
#endif
#ifdef ENABLE_I2C
#include "drivers/i2c.hpp"
#endif
#ifdef ENABLE_SPI
#include "drivers/spi.hpp"
#endif
#ifdef ENABLE_MONOCOLOUR_LED
#include "drivers/led/monocolour.hpp"
#endif
#ifdef ENABLE_TTP223
#include "drivers/touch/ttp223.hpp"
#endif
#ifdef ENABLE_PCF8563
#include "drivers/rtc/pcf8563.hpp"
#endif
#ifdef ENABLE_FAKE_RTC
#include "drivers/rtc/fake.hpp"
#endif
#ifdef ENABLE_MPU9250
#include "drivers/accellerometer/mpu9250.hpp"
#endif
#ifdef ENABLE_ST7735
#include "drivers/display/st7735.hpp"
#endif
#ifdef ENABLE_ST7789
#include "drivers/display/st7789.hpp"
#endif

// System Services
#ifdef ENABLE_SERVICE_NTP
#include "services/ntp.hpp"
#endif
#ifdef ENABLE_SERVICE_SLEEPTUNE
#include "services/sleeptune.hpp"
#endif
#ifdef ENABLE_CPU_SCALER
#include "services/cpu_scaler.hpp"
#endif

using namespace std;
using driverList = std::list<sDOS_Abstract_Driver *>;
using serviceList = std::list<sDOS_Abstract_Service *>;


class sDOS
{
public:
    sDOS();
    void Setup();
    void Loop();
    //static uint64_t getLoopCount();

private:
    String _component = "Kernel";
    driverList _drivers;
    serviceList _services;
    void _configure();
    Preferences _preferences;
    Debugger _debugger = Debugger();
    FileSystem _fileSystem = FileSystem(_debugger);
    EventsManager _events = EventsManager(_debugger);
    WiFiManager * _driver_WiFi = new WiFiManager(_debugger, _fileSystem, _events);
    BluetoothManager * _driver_BT = new BluetoothManager(_debugger, _events);
    #ifdef ENABLE_CPU_SCALER
    SDOS_CPU_SCALER *_cpuScaler = new SDOS_CPU_SCALER(_debugger, _events, _driver_WiFi, _driver_BT);
    #endif
    long _lastCycleTimeMS = 0;
    long _lastTimeStampUS = 0;
};


sDOS::sDOS(){

};

void sDOS::Setup()
{
#if defined(ENABLE_CPU_SCALER)
    setCpuFrequencyMhz(20);
#endif
    Serial.begin(SERIAL_BAUD);
    Serial.setDebugOutput(SERIAL_DEBUG_ENABLE);
    delay(300);

#if defined(ENABLE_POWER)
    _drivers.push_back(new SDOS_POWER(_debugger, _events));
#endif

    _debugger.Debug(_component, F("Started Smol Device Operating System Kernel"));
    _debugger.Debug(_component, F("Built with love on %s at %s."), __DATE__, __TIME__);

    _drivers.push_back(_driver_WiFi);
    _drivers.push_back(_driver_BT);

#if defined(ENABLE_I2C)
    SDOS_I2C * _driver_I2C = new SDOS_I2C(_debugger, _events);
    _drivers.push_back(_driver_I2C);
#endif
#if defined(ENABLE_SPI)
    SDOS_SPI * _driver_SPI = new SDOS_SPI(_debugger, _events);
    _drivers.push_back(_driver_SPI);
#endif
#if defined(ENABLE_MONOCOLOUR_LED)
    _drivers.push_back(new SDOS_LED_MONO(_debugger, _events, ENABLE_MONOCOLOUR_LED));
#endif
#if defined(ENABLE_TTP223)
    _drivers.push_back(new SDOS_TTP223(_debugger, _events));
#endif
#if defined(ENABLE_PCF8563) && defined(ENABLE_I2C)
#define ENABLE_RTC
    SDOS_PCF8563 * _driver_RTC = new SDOS_PCF8563(_debugger, _events, _driver_I2C);
    _drivers.push_back(_driver_RTC);
#endif
#if defined(ENABLE_FAKE_RTC)
#define ENABLE_RTC
    SDOS_FAKE_RTC * _driver_RTC = new SDOS_FAKE_RTC(_debugger, _events);
#endif
#if defined(ENABLE_MPU9250)
    _drivers.push_back(new SDOS_MPU9250(_debugger, _events));
#endif
#if defined(ENABLE_ST7735) && defined(ENABLE_SPI)
#define ENABLE_DISPLAY
    _drivers.push_back(new SDOS_DISPLAY_ST7735(_debugger, _events, _driver_SPI));
#endif
#if defined(ENABLE_ST7789) && defined(ENABLE_SPI)
#define ENABLE_DISPLAY
    _drivers.push_back(new SDOS_DISPLAY_ST7789(_debugger, _events, _driver_SPI));
#endif

#if defined(ENABLE_CPU_SCALER)
    _services.push_back(_cpuScaler);
#endif
#if defined(ENABLE_SERVICE_SLEEPTUNE)
    _services.push_back(new SDOS_SLEEPTUNE(_debugger, _events, _driver_WiFi, _driver_BT));
#endif
#if defined(ENABLE_SERVICE_NTP) && defined(ENABLE_RTC)
    _services.push_back(new SDOS_NTP(_debugger, _events, _driver_RTC, _driver_WiFi));
#endif

    // Setup Drivers
    for (auto const& it : _drivers) {
        #ifdef DEBUG_LOOP_RUNNING
        _debugger.Debug(_component, ">>> Setup -> Driver -> %s", it->getName());
        #endif
        it->setup();
        #ifdef DEBUG_LOOP_RUNNING
        _debugger.Debug(_component, "<<< Setup -> Driver -> %s", it->getName());
        #endif
    }

    // Setup Services
    for (auto const& it : _services) {
        #ifdef DEBUG_LOOP_RUNNING
        _debugger.Debug(_component, ">>> Setup -> Service -> %s", it->getName());
        #endif
        it->setup();
        #ifdef DEBUG_LOOP_RUNNING
        _debugger.Debug(_component, "<<< Setup -> Service -> %s", it->getName());
        #endif
    }

#if defined(ENABLE_CPU_SCALER)
    // To slow down the clock sooner rather than later, we call CPU_SCALERS loop here as an extra.
    _cpuScaler->loop();
#endif
};

void sDOS::Loop()
{

    // Calculate how long it takes to iterate a loop.
    long microseconds = micros();
    long _lastCycleTimeUS = microseconds - _lastTimeStampUS;
    _lastCycleTimeMS = _lastCycleTimeUS / 1000;

    _lastTimeStampUS = microseconds;
    _loopCount++;
    if(_loopCount > 10000){
        _loopCount = 0;
    }

    // Loop over Drivers
    for (auto const& it : _drivers) {
        if(it->isActive()){
            #ifdef DEBUG_LOOP_RUNNING
            _debugger.Debug(_component, ">>> Loop -> Driver -> %s", it->getName());
            #endif
            it->loop();
            #ifdef DEBUG_LOOP_RUNNING
            _debugger.Debug(_component, "<<< Loop -> Driver -> %s", it->getName());
            #endif
        }else{
            #ifdef DEBUG_LOOP_RUNNING
            _debugger.Debug(_component, "xxx Loop -> Driver -> %s", it->getName());
            #endif
        }
    }

    // Loop over Services
    for (auto const& it : _services) {
        if(it->isActive()){
            #ifdef DEBUG_LOOP_RUNNING
            _debugger.Debug(_component, ">>> Loop -> Service -> %s", it->getName());
            #endif
            it->loop();
            #ifdef DEBUG_LOOP_RUNNING
            _debugger.Debug(_component, "<<< Loop -> Service -> %s", it->getName());
            #endif
        }else{
            #ifdef DEBUG_LOOP_RUNNING
            _debugger.Debug(_component, "xxx Loop -> Service -> %s", it->getName());
            #endif

        }
    }
    
    // Check the Events loop
    _events.loop();
    yield();
}

void Debugger::Debug(String component, String format, ...)
{
    char buff[DEBUG_SERIAL_BUFFER_SIZE];
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
        "%s[%s%04d %s%.7-7s %s%dMhz %s%s %s%s %s%dmV%s] %s\n", 
        Debugger::duplicates > 0 ? "\n":"", 
        COL_BLUE,
        _loopCount,
        COL_YELLOW,
        component.c_str(), 
        getCpuFrequencyMhz() > 20 ? COL_RED : COL_GREEN,
        getCpuFrequencyMhz(),
        WiFi.isConnected() ? COL_RED : COL_GREEN,
        WiFi.isConnected() ? "W+" : "W-",
        sdos_is_bluetooth_active() ? COL_RED : COL_GREEN,
        sdos_is_bluetooth_active() ? "B+" : "B-",
        SDOS_POWER::isCharging() ? COL_BLUE : COL_PINK,
        SDOS_POWER::getVbattMv(),
        COL_RESET,
        buff
    );

    _serial.print(outputBuffer);

    for (auto const& it : Debugger::_handlers) {
        it(outputBuffer);
    }

    memcpy(Debugger::lastBuff, buff, sizeof(Debugger::lastBuff));
    Debugger::lastComponent = component;
    Debugger::duplicates = 0;
    return;
};

#endif