#pragma once
#include "kern_inc.h"
#include <algorithm>
#include <iostream>
#include <list>

#define DEBUG_SERIAL_BUFFER_SIZE 255

using namespace std;
using debugHandlersList = std::list<void (*)(String message)>;

#ifndef SDOS_SERIAL_DEBUG_UNDERLYING_SYSTEM_DEBUG
#define SDOS_SERIAL_DEBUG_UNDERLYING_SYSTEM_DEBUG false
#endif

class Debugger {
protected:
    HardwareSerial _serial = Serial;
    static String _lastComponent;
    static char _lastBuff[DEBUG_SERIAL_BUFFER_SIZE];
    static int _duplicates;
    static debugHandlersList _handlers;
    static bool isBluetoothPoweredOn();
    static bool isWifiPoweredOn();
    static bool isPowerCharging();
    static double getBatteryVolts();
public:
    Debugger() : _serial(Serial){};

    void Debug(String component, const String& format, ...) {
        char buff[DEBUG_SERIAL_BUFFER_SIZE];
        va_list args;
        va_start(args, format.c_str());
        vsprintf(buff, format.c_str(), args);

        if (Debugger::_lastComponent.equals(component) && strcmp(buff, Debugger::_lastBuff) == 0) {
            Debugger::_duplicates++;
            _serial.printf("\t\t(repeated %d times)\r", Debugger::_duplicates);
            return;
        }

        component.toUpperCase();

        char outputBuffer[sizeof(buff)];
        _serial.print(Debugger::getDuplicates() > 0 ? "\n" : "");
        _serial.print("[");
        _serial.printf("%s%04d ", COL_BLUE, _loopCount);
        _serial.printf("%s%-7.7s ", COL_YELLOW, component.c_str());
        _serial.printf("%s%3dMhz ", getCpuFrequencyMhz() > 20 ? COL_RED : COL_GREEN, getCpuFrequencyMhz());
        _serial.printf("%s%s ",Debugger::isWifiPoweredOn() ? COL_RED : COL_GREEN, Debugger::isWifiPoweredOn() ? F("W+") : F("W-"));
#if defined(ENABLE_BLUETOOTH) && defined(ESP32)
        _serial.printf("%s%s ", Debugger::isBluetoothPoweredOn() ? COL_RED : COL_GREEN, Debugger::isBluetoothPoweredOn() ? F("B+") : F("B-"));
#endif
#if defined(ENABLE_POWER)
        _serial.printf("%s%.2fv ", Debugger::isPowerCharging() ? COL_BLUE : COL_PINK, Debugger::getBatteryVolts());
#endif
        _serial.printf("%s%3dK", ESP.getFreeHeap() < 50000 ? COL_RED : COL_BLUE, ESP.getFreeHeap() / 1024);
        _serial.printf("%s] ", COL_RESET);
        _serial.println(buff);
        _serial.flush();

        for (auto const &it : Debugger::_handlers) {
            it(outputBuffer);
        }

        memcpy(Debugger::_lastBuff, buff, sizeof(Debugger::_lastBuff));
        Debugger::_lastComponent = component;
        Debugger::_duplicates = 0;
        return;
    };

    static void addHandler(void (*newHandler)(String message)) {
        Debugger::_handlers.push_back(newHandler);
    };
    HardwareSerial getSerial() {
        return _serial;
    }
    static int getDuplicates() {
        return Debugger::_duplicates;
    };
};

String Debugger::_lastComponent;
char Debugger::_lastBuff[DEBUG_SERIAL_BUFFER_SIZE];
int Debugger::_duplicates = 0;
debugHandlersList Debugger::_handlers;
