#pragma once

#include "kern_inc.h"

unsigned int _loopCount = 0;

#include <colours.h>
#include <debugger.hpp>
#include <events.hpp>
#include <drivers/filesystem.hpp>
#include <drivers/WiFiManager.hpp>

#ifdef ESP32
#include <drivers/BluetoothManager.hpp>
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
#ifdef ENABLE_AXP192
#include "drivers/pwrmgmt/axp192.hpp"
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
#if defined(ENABLE_ST7735) && defined(ENABLE_SPI)
#pragma message "Support for ST7735 enabled"
#define ENABLE_DISPLAY
#include "drivers/display/st7735.hpp"
#include "drivers/display/frame_buffer.hpp"
#endif
#if defined(ENABLE_ST7789) && defined(ENABLE_SPI)
#pragma message "Support for ST7789 enabled"
#define ENABLE_DISPLAY
#include "drivers/display/st7789.hpp"
#include "drivers/display/frame_buffer.hpp"
#endif
#if defined(ENABLE_DISPLAY)
#pragma message "Display enabled"
#endif

// System Services
#include <abstracts/service.hpp>

#ifdef ENABLE_SERVICE_NTP
#include "services/ntp.hpp"
#endif
#ifdef ENABLE_SERVICE_OTA
#include "services/ota.hpp"
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

class sDOS {
public:
    sDOS() {
        Serial.println("sDOS::sDOS()");
    }

    void setup();

    void Loop();

    void add(sDOS_Abstract_Service *service) {
        this->_debugger->Debug(this->_component, "Adding %s service", service->getName());
        this->_services.push_back(service);
    }

    void add(sDOS_Abstract_Driver *driver) {
        this->_debugger->Debug(this->_component, "Adding %s driver", driver->getName());
        this->_drivers.push_back(driver);
    }

protected:
    String _component = "Kernel";
    uint64_t _chip_id;
    driverList _drivers;
    serviceList _services;

    Debugger *_debugger;
    EventsManager * _eventsManager;
    FileSystem *_fileSystem;

    WiFiManager *_driver_WiFi;
#if defined(ENABLE_BLUETOOTH) && defined(ESP32)
    BluetoothManager *_driver_BT;
#endif
#if defined(ENABLE_CPU_SCALER) && defined(ESP32)
    sDOS_CPU_SCALER *_cpuScaler;
#endif

#if defined(ENABLE_RTC) && defined(ENABLE_PCF8563) && defined(ENABLE_I2C)
    sDOS_PCF8563 * _driver_RTC;
#elif defined(ENABLE_RTC) && defined(ENABLE_FAKE_RTC)
    sDOS_FAKE_RTC * _driver_RTC;
#endif
#if defined(ENABLE_DISPLAY) && defined(ESP32)
    AbstractDisplay * _display;
    sDOS_FrameBuffer * _driver_FrameBuffer;
#endif
#ifdef ENABLE_AXP192
    sDOS_AXP192 * _axp192;
#endif

#if defined(ENABLE_BUTTON)
    sDOS_BUTTON * _button;
#endif
#if defined(ENABLE_TTP223)
    sDOS_TTP223 * _button_ttp223;
#endif
#if defined(ENABLE_MONOCOLOUR_LED)
    sDOS_LED_MONO * _mono_led;
#endif

    long _lastCycleTimeMS = 0;
    long _lastTimeStampUS = 0;
};

void sDOS::setup() {
    this->_debugger = new Debugger();
    this->_eventsManager = new EventsManager(this->_debugger);
    this->_fileSystem = new FileSystem(this->_debugger, this->_eventsManager);

#if defined(ENABLE_CPU_SCALER) && defined(ESP32)
    this->_cpuScaler = new sDOS_CPU_SCALER(_debugger, this->_eventsManager, _driver_WiFi, _driver_BT);
#endif
#if defined(ENABLE_BLUETOOTH) && defined(ESP32)
    this->_driver_BT = new BluetoothManager(_debugger, this->_eventsManager);
#endif
    this->_driver_WiFi = new WiFiManager(this->_debugger, this->_fileSystem, this->_eventsManager);
#if defined(ENABLE_BUTTON)
    //this->_button = new sDOS_BUTTON(this->_debugger, this->_eventsManager);
#endif
#if defined(ENABLE_TTP223)
    //this->_button_ttp223 = new sDOS_TTP223(this->_debugger, this->_eventsManager);
#endif
#if defined(ENABLE_MONOCOLOUR_LED)
    this->_mono_led = new sDOS_LED_MONO(this->_debugger, this->_eventsManager, ENABLE_MONOCOLOUR_LED);
#endif

#if defined(DEBUG_EVENTS)
    this->_debugger->Debug(_component, F("Compiled with DEBUG_EVENTS enabled"));
#endif
#if defined(DEBUG_LOOP_RUNNING)
    this->_debugger->Debug(_component, F("Compiled with DEBUG_LOOP_RUNNING enabled"));
#endif
#if defined(DEBUG_CPU_SCALER_DECISIONS)
    this->_debugger->Debug(_component, F("Compiled with DEBUG_CPU_SCALER_DECISIONS enabled"));
#endif
#if defined(DEBUG_WIFIMANAGER_ISACTIVE_DECISIONS)
    this->_debugger->Debug(_component, F("Compiled with DEBUG_WIFIMANAGER_ISACTIVE_DECISIONS enabled"));
#endif

#if defined(ENABLE_CPU_SCALER) && defined(ESP32)
    setCpuFrequencyMhz(20);
#endif
    this->add(_fileSystem);

#if defined(ENABLE_POWER)
    this->add(new sDOS_POWER(this->_debugger, this->_eventsManager));
#endif

    this->_debugger->Debug(_component, F("Started %sSmol Device Operating System%s Kernel"), COL_GREEN, COL_RESET);
    this->_debugger->Debug(_component, F("Built with %slove%s on %s at %s."), COL_RED, COL_RESET, F(__DATE__), F(__TIME__));
#ifdef ESP32
    _chip_id = ESP.getEfuseMac();
    this->_debugger->Debug(_component, F("Hardware ID: %s%04X%08X%s"), COL_PINK, (uint16_t) (_chip_id >> 32),
                    (uint32_t) (_chip_id), COL_RESET);
#endif
#ifdef ESP8266
    _chip_id = ESP.getChipId();
    this->_debugger->Debug(_component, F("Hardware ID: %s%08X%s"), COL_PINK, (uint32_t)(_chip_id), COL_RESET);
#endif

#if defined(ENABLE_I2C)
    delay(1);
    sDOS_I2C * _driver_I2C = new sDOS_I2C(this->_debugger, _eventsManager);
    this->add(_driver_I2C);
#endif
#if defined(ENABLE_SPI)
    sDOS_SPI * _driver_SPI = new sDOS_SPI(this->_debugger, this->_eventsManager);
    this->add(_driver_SPI);
#endif
#if defined(ENABLE_ST7735) && defined(ENABLE_SPI)
    _display = new sDOS_DISPLAY_ST7735(this->_debugger, this->_eventsManager, _driver_SPI);
    this->add(_display);
#endif
#if defined(ENABLE_ST7789) && defined(ENABLE_SPI)
    _display = new sDOS_DISPLAY_ST7789(this->_debugger, this->_eventsManager, _driver_SPI);
    this->add(_display);
#endif
#ifdef ENABLE_AXP192
    _axp192 = new sDOS_AXP192(this->_debugger, this->_eventsManager, _driver_I2C);
    this->add(_axp192);
#endif
#if defined(ENABLE_MONOCOLOUR_LED)
    this->add(_mono_led);
#endif
#if defined(ENABLE_BUTTON)
    this->_debugger->Debug(_component, "ENABLE BUTTON");
    //this->add(_button);
#endif
#if defined(ENABLE_TTP223)
    this->add(_button_ttp223);
#endif
#if defined(ENABLE_PCF8563) && defined(ENABLE_RTC) && defined(ENABLE_I2C)
    _driver_RTC = new sDOS_PCF8563(this->_debugger, this->_eventsManager, _driver_I2C);
    this->add(_driver_RTC);
#endif
#if defined(ENABLE_FAKE_RTC) && defined(ENABLE_RTC)
    _driver_RTC = new sDOS_FAKE_RTC(this->_debugger, this->_eventsManager);
    this->add(_driver_RTC);
#endif
#if defined(ENABLE_MPU9250)
    this->_debugger->Debug(_component, "ENABLE MPU9250");
    this->add(new sDOS_MPU9250(this->_debugger, this->_eventsManager));
#endif
#if defined(ENABLE_DISPLAY) && defined(ESP32)
    _driver_FrameBuffer = new sDOS_FrameBuffer(this->_debugger, this->_eventsManager, _display, this->_cpuScaler);
    _driver_FrameBuffer->init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    this->add(_driver_FrameBuffer);
#endif

#if defined(ENABLE_WIFI)
    this->add(_driver_WiFi);
#endif
#if defined(ENABLE_BLUETOOTH) && defined(ESP32)
    this->add(_driver_BT);
#endif
#if defined(ENABLE_CPU_SCALER) && defined(ESP32)
    this->add(this->_cpuScaler);
#endif
#if defined(ENABLE_SERVICE_SLEEPTUNE) && defined(ESP32)
    this->add(new sDOS_SLEEPTUNE(this->_debugger, this->_eventsManager, this->_driver_WiFi, this->_driver_BT));
#endif
#if defined(ENABLE_SERVICE_NTP) && defined(ENABLE_RTC)
    this->add(new sDOS_NTP(this->_debugger, this->_eventsManager, this->_driver_RTC, this->_driver_WiFi));
#endif

#if defined(ENABLE_CPU_SCALER) && defined(ESP32)
    // To slow down the clock sooner rather than later, we call CPU_SCALERS loop here as an extra.
    this->_cpuScaler->loop();
#endif

    // Loop over Drivers

    for (auto const &it : this->_drivers) {
        this->_debugger->Debug(_component, "calling driver %s->setup()", it->getName());
        it->setup();
        yield();
    }

    // Loop over Services
    for (auto const &it : this->_services) {
        this->_debugger->Debug(_component, "calling service %s->setup()", it->getName());
        it->setup();
        yield();
    }

    this->_debugger->Debug(_component, "%s*** Setup Complete, Runtime Begins ***%s", COL_GREEN, COL_RESET);
};

void sDOS::Loop() {
#ifdef DEBUG_LOOP_RUNNING
    this->_debugger->Debug(_component, "%s>>> Loop Start %s", COL_GREEN, COL_RESET);
#endif

    // Calculate how long it takes to iterate a loop.
    long microseconds = micros();
    long _lastCycleTimeUS = microseconds - _lastTimeStampUS;
    _lastCycleTimeMS = _lastCycleTimeUS / 1000;

    _lastTimeStampUS = microseconds;
    _loopCount++;
    if (_loopCount > 10000) {
        _loopCount = 0;
    }

    // Loop over Drivers
    for (auto const &it : _drivers) {
        if (it->isActive()) {
            uint64_t started = micros();
#ifdef DEBUG_LOOP_RUNNING
            this->_debugger->Debug(_component, "%s>>> Loop -> Driver -> %s%s", COL_GREEN, it->getName().c_str(), COL_RESET);
#endif
            it->loop();
            uint64_t runTimeMS = (micros() - started) / 1000;
#ifdef DEBUG_LOOP_RUNNING
            this->_debugger->Debug(_component, "%s<<< Loop <- Driver <- %s%s (in %dms)", COL_GREEN, it->getName().c_str(), COL_RESET, runTimeMS);
#else
#endif
        } else {
#ifdef DEBUG_LOOP_RUNNING
            this->_debugger->Debug(_component, "%sxxx SKIP >< Driver >< %s%s", COL_RED, it->getName().c_str(), COL_RESET);
#endif
        }
        yield();
    }

    // Loop over Services
    for (auto const &it : _services) {
        if (it->isActive()) {
            uint64_t started = micros();
#ifdef DEBUG_LOOP_RUNNING
            this->_debugger->Debug(_component, "%s>>> Loop -> Service -> %s%s", COL_GREEN, it->getName().c_str(), COL_RESET);
#endif
            it->loop();
            uint64_t runTimeMS = (micros() - started) / 1000;
#ifdef DEBUG_LOOP_RUNNING
            this->_debugger->Debug(_component, "%s<<< Loop <- Service <- %s%s (in %dms)", COL_GREEN, it->getName().c_str(), COL_RESET, runTimeMS);
#endif
        } else {
#ifdef DEBUG_LOOP_RUNNING
            this->_debugger->Debug(_component, "%sxxx SKIP >< Service >< %s%s", COL_GREEN, it->getName().c_str(), COL_RESET);
#endif
        }
        yield();
    }

    // Check the Events loop
#ifdef DEBUG_LOOP_RUNNING
    this->_debugger->Debug(_component, "%s>>> Loop -> Events Manager %s", COL_GREEN, COL_RESET);
#endif
    this->_eventsManager->loop();
    yield();
#ifdef DEBUG_LOOP_RUNNING
    this->_debugger->Debug(_component, "%s<<< Loop <- Events Manager %s", COL_GREEN, COL_RESET);
#endif

#ifdef DEBUG_LOOP_RUNNING
    this->_debugger->Debug(_component, "%s<<< Loop Complete %s", COL_GREEN, COL_RESET);
#endif
};