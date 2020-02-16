#ifndef SDOS_DEBUGGER_HPP
#define SDOS_DEBUGGER_HPP
#include "kern_inc.h"
#include <algorithm>
#include <iostream>
#include <list>
#define DEBUG_SERIAL_BUFFER_SIZE 255

#ifndef SDOS_SERIAL_DEBUG_ENABLED
#define SDOS_SERIAL_DEBUG_ENABLED true
#endif

bool sdos_is_wifi_active();
#if defined(BLUETOOTH_ENABLED) && defined(ESP32)
bool sdos_is_bluetooth_active();
#endif

#if SDOS_SERIAL_DEBUG_ENABLED == true
    using namespace std;
    using handlersList = std::list<void(*)(String message)>;
#endif

#ifndef SDOS_SERIAL_DEBUG_UNDERLYING_SYSTEM_DEBUG
#define SDOS_SERIAL_DEBUG_UNDERLYING_SYSTEM_DEBUG false
#endif

class Debugger
{
    public:
        Debugger()
        {
            #if SDOS_SERIAL_DEBUG_ENABLED == true
                _serial = Serial;
                _serial.begin(SERIAL_BAUD);
                _serial.setDebugOutput(SDOS_SERIAL_DEBUG_UNDERLYING_SYSTEM_DEBUG);
            #endif
        };
        void Debug(String component, String message, ...);
        unsigned int Step(){
            #if SDOS_SERIAL_DEBUG_ENABLED == true
                Debugger::_stepCount++;
                Debugger::Debug("step", "%d", Debugger::_stepCount);
                return Debugger::_stepCount;
            #endif
        };
        void addHandler(void (*newHandler)(String message))
        {
            #if SDOS_SERIAL_DEBUG_ENABLED == true
                Debugger::_handlers.push_back(newHandler);
            #endif
        };

    private:
            #if SDOS_SERIAL_DEBUG_ENABLED == true
            HardwareSerial _serial = Serial;
            static String lastComponent;
            static char lastBuff[DEBUG_SERIAL_BUFFER_SIZE];
            static int duplicates;
            static handlersList  _handlers;
            static unsigned int _stepCount;
        #endif
};
#if SDOS_SERIAL_DEBUG_ENABLED == true
    String Debugger::lastComponent;
    char Debugger::lastBuff[DEBUG_SERIAL_BUFFER_SIZE];
    int Debugger::duplicates = 0;
    handlersList Debugger::_handlers;
    unsigned int Debugger::_stepCount;
#endif

#endif