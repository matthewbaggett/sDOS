#ifndef kern_hpp
#define kern_hpp
#include "kern_inc.h"

unsigned int _loopCount = 0;

#include <colours.h>
#include <debugger.hpp>
#include <events.hpp>
#include <filesystem.hpp>
#include <wifi.hpp>

#ifdef ESP32
#include <bluetooth/bluetooth.hpp>
#endif

#include <abstracts/driver.hpp>
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
#ifdef ENABLE_BUTTON
#include "drivers/touch/button.hpp"
#endif
#ifdef ENABLE_TTP223
#include "drivers/touch/ttp223.hpp"
#endif
#ifdef ENABLE_PCF8563
#define ENABLE_RTC
#include "drivers/rtc/pcf8563.hpp"
#endif
#ifdef ENABLE_FAKE_RTC
#define ENABLE_RTC
#include "drivers/rtc/fake.hpp"
#endif
#ifdef ENABLE_MPU9250
#define ENABLE_ACCELLEROMETER
#include "drivers/accellerometer/mpu9250.hpp"
#endif
#ifdef ENABLE_ST7735
#define ENABLE_DISPLAY
#include "drivers/display/st7735.hpp"
#include "drivers/display/frame_buffer.hpp"
#endif
#ifdef ENABLE_ST7789
#define ENABLE_DISPLAY
#include "drivers/display/st7789.hpp"
#include "drivers/display/frame_buffer.hpp"
#endif

// System Services
#include <abstracts/service.hpp>
#ifdef ENABLE_SERVICE_NTP
#include "services/ntp.hpp"
#endif
#if defined(ENABLE_SERVICE_SLEEPTUNE) && defined(ESP32)
#include "services/sleeptune.hpp"
#endif
#if defined(ENABLE_CPU_SCALER) && defined(ESP32)
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
        void addService(sDOS_Abstract_Service * service);

    protected:
        String _component = "Kernel";
        uint64_t _chip_id;
        driverList _drivers;
        serviceList _services;
        void _configure();
        Debugger _debugger = Debugger();
        FileSystem _fileSystem = FileSystem(_debugger);
        EventsManager _events = EventsManager(_debugger);
        WiFiManager * _driver_WiFi = new WiFiManager(_debugger, _fileSystem, _events);
        #ifdef ESP32
        BluetoothManager * _driver_BT = new BluetoothManager(_debugger, _events);
        #endif
        #if defined(ENABLE_RTC)
        AbstractRTC * _driver_RTC;
        #endif
        #if defined(ENABLE_DISPLAY) && defined(ESP32)
        AbstractDisplay * _display;
        sDOS_FrameBuffer * _driver_FrameBuffer;
        #endif
        #if defined(ENABLE_CPU_SCALER) && defined(ESP32)
        sDOS_CPU_SCALER *_cpuScaler = new sDOS_CPU_SCALER(_debugger, _events, _driver_WiFi, _driver_BT);
        #endif
        long _lastCycleTimeMS = 0;
        long _lastTimeStampUS = 0;
};


sDOS::sDOS(){

};

void sDOS::addService(sDOS_Abstract_Service * service){
    _services.push_back(service);
}

void sDOS::Setup()
{
#if defined(ENABLE_CPU_SCALER) && defined(ESP32)
    setCpuFrequencyMhz(20);
#endif

#if defined(ENABLE_POWER)
    _drivers.push_back(new sDOS_POWER(_debugger, _events));
#endif

    _debugger.Debug(_component, F("Started %sSmol Device Operating System%s Kernel"), COL_GREEN, COL_RESET);
    _debugger.Debug(_component, F("Built with %slove%s on %s at %s."),COL_RED, COL_RESET, F(__DATE__), F(__TIME__));
    #ifdef ESP32
    _chip_id = ESP.getEfuseMac();
    _debugger.Debug(_component, F("Hardware ID: %s%04X%08X%s"), COL_PINK, (uint16_t)(_chip_id>>32), (uint32_t)(_chip_id), COL_RESET);
    #endif
    #ifdef ESP8266
    _chip_id = ESP.getChipId();
    _debugger.Debug(_component, F("Hardware ID: %s%08X%s"), COL_PINK, (uint32_t)(_chip_id), COL_RESET);
    #endif

#if defined(ENABLE_I2C)
    delay(1);
    sDOS_I2C * _driver_I2C = new sDOS_I2C(_debugger, _events);
    _drivers.push_back(_driver_I2C);
#endif
#if defined(ENABLE_SPI)
    sDOS_SPI * _driver_SPI = new sDOS_SPI(_debugger, _events);
    _drivers.push_back(_driver_SPI);
#endif
#if defined(ENABLE_ST7735) && defined(ENABLE_SPI)
    _display = new sDOS_DISPLAY_ST7735(_debugger, _events, _driver_SPI);
    _drivers.push_back(_display);
#endif
#if defined(ENABLE_ST7789) && defined(ENABLE_SPI)
    _display = new sDOS_DISPLAY_ST7789(_debugger, _events, _driver_SPI);
    _drivers.push_back(_display);
#endif
#if defined(ENABLE_MONOCOLOUR_LED)
    _drivers.push_back(new sDOS_LED_MONO(_debugger, _events, ENABLE_MONOCOLOUR_LED));
#endif
#ifdef ENABLE_BUTTON
    _drivers.push_back(new sDOS_BUTTON(_debugger, _events));
#endif
#if defined(ENABLE_TTP223)
    _drivers.push_back(new sDOS_TTP223(_debugger, _events));
#endif
#if defined(ENABLE_PCF8563) && defined(ENABLE_RTC) && defined(ENABLE_I2C)
    _driver_RTC = new sDOS_PCF8563(_debugger, _events, _driver_I2C);
    _drivers.push_back(_driver_RTC);
#endif
#if defined(ENABLE_FAKE_RTC) && defined(ENABLE_RTC) 
    _driver_RTC = new sDOS_FAKE_RTC(_debugger, _events);
    _drivers.push_back(_driver_RTC);
#endif
#if defined(ENABLE_MPU9250)
    _drivers.push_back(new sDOS_MPU9250(_debugger, _events));
#endif
#if defined(ENABLE_DISPLAY)
    _driver_FrameBuffer = new sDOS_FrameBuffer(_debugger, _events, _display, _cpuScaler);
    _driver_FrameBuffer->init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    _drivers.push_back(_driver_FrameBuffer);
#endif

#if defined(ENABLE_WIFI)
    _drivers.push_back(_driver_WiFi);
#endif
#if defined(ENABLE_BLUETOOTH) && defined(ESP32)
    _drivers.push_back(_driver_BT);
#endif

#if defined(ENABLE_CPU_SCALER) && defined(ESP32)
    _services.push_back(_cpuScaler);
#endif
#if defined(ENABLE_SERVICE_SLEEPTUNE) && defined(ESP32)
    _services.push_back(new sDOS_SLEEPTUNE(_debugger, _events, _driver_WiFi, _driver_BT));
#endif
#if defined(ENABLE_SERVICE_NTP) && defined(ENABLE_RTC)
    _services.push_back(new sDOS_NTP(_debugger, _events, _driver_RTC, _driver_WiFi));
#endif

    // Setup Drivers
    for (auto const& it : _drivers) {
        #ifdef DEBUG_LOOP_RUNNING
        _debugger.Debug(_component, "%s>>> Setup -> Driver -> %s%s", COL_GREEN, it->getName().c_str(), COL_RESET);
        #endif
        it->setup();
        #ifdef DEBUG_LOOP_RUNNING
        _debugger.Debug(_component, "%s<<< Setup -> Driver -> %s%s", COL_GREEN, it->getName().c_str(), COL_RESET);
        #endif
        yield();
    }

    // Setup Services
    for (auto const& it : _services) {
        #ifdef DEBUG_LOOP_RUNNING
        _debugger.Debug(_component, "%s>>> Setup -> Service -> %s%s", COL_GREEN, it->getName().c_str(), COL_RESET);
        #endif
        it->setup();
        #ifdef DEBUG_LOOP_RUNNING
        _debugger.Debug(_component, "%s<<< Setup -> Service -> %s%s", COL_GREEN, it->getName().c_str(), COL_RESET);
        #endif
        yield();
    }

#if defined(ENABLE_CPU_SCALER) && defined(ESP32)
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
            uint64_t started = micros();
            _debugger.Debug(_component, "%s>>> Loop -> Driver -> %s%s", COL_GREEN, it->getName().c_str(), COL_RESET);
            #endif
            it->loop();
            #ifdef DEBUG_LOOP_RUNNING
            _debugger.Debug(_component, "%s<<< Loop <- Driver <- %s%s (in %dms)", COL_GREEN, it->getName().c_str(), COL_RESET, (micros() - started) / 1000);
            #endif
        }else{
            #ifdef DEBUG_LOOP_RUNNING
            _debugger.Debug(_component, "%sxxx SKIP >< Driver >< %s%s", COL_RED, it->getName().c_str(), COL_RESET);
            #endif
        }
        yield();
    }

    // Loop over Services
    for (auto const& it : _services) {
        if(it->isActive()){
            #ifdef DEBUG_LOOP_RUNNING
                uint64_t started = micros();
            _debugger.Debug(_component, "%s>>> Loop -> Service -> %s%s", COL_GREEN, it->getName().c_str(), COL_RESET);
            #endif
            it->loop();
            #ifdef DEBUG_LOOP_RUNNING
            _debugger.Debug(_component, "%s<<< Loop <- Service <- %s%s (in %dms)", COL_GREEN, it->getName().c_str(), COL_RESET, (micros() - started) / 1000);
            #endif
        }else{
            #ifdef DEBUG_LOOP_RUNNING
            _debugger.Debug(_component, "%sxxx SKIP >< Service >< %s%s", COL_GREEN, it->getName().c_str(), COL_RESET);
            #endif
        }
        yield();
    }
    
    // Check the Events loop
    _events.loop();
    yield();
}

void Debugger::Debug(String component, String format, ...)
{
    #if defined(SDOS_SERIAL_DEBUG_ENABLED) && SDOS_SERIAL_DEBUG_ENABLED == true
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
            "%s[%s%04d %s%.7-7s %s%dMhz %s%s %s%s %s%.1fV %s%dK%s] %s\n",
            Debugger::duplicates > 0 ? "\n":"",
            COL_BLUE,
            _loopCount,
            COL_YELLOW,
            component.c_str(),
            getCpuFrequencyMhz() > 20 ? COL_RED : COL_GREEN,
            getCpuFrequencyMhz(),
            WiFi.isConnected() ? COL_RED : COL_GREEN,
            WiFi.isConnected() ? "W+" : "W-",
            #if defined(ENABLE_BLUETOOTH) && defined(ESP32)
            sdos_is_bluetooth_active() ? COL_RED : COL_GREEN,
            sdos_is_bluetooth_active() ? "B+" : "B-",
            #else
            NULL, NULL,
            #endif
            #if defined(ENABLE_POWER)
            sDOS_POWER::isCharging() ? COL_BLUE : COL_PINK,
            ((float) sDOS_POWER::getVbattMv()) / 1000,
            #else
            NULL, NULL,
            #endif
            COL_BLUE,
            ESP.getFreeHeap()/1024,
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
    #endif
};

#endif