#ifndef SDOS_DEBUGGER_HPP
#define SDOS_DEBUGGER_HPP
#include "kern_inc.h"
#include <algorithm>
#include <iostream>
#include <list>
#define DEBUG_SERIAL_BUFFER_SIZE 255

bool sdos_is_wifi_active();
#if defined(BLUETOOTH_ENABLED) && defined(ESP32)
bool sdos_is_bluetooth_active();
#endif

using namespace std;
using handlersList = std::list<void(*)(String message)>;


class Debugger
{
    public:
        Debugger()
        {
            _serial = Serial;
            _serial.begin(SERIAL_BAUD);
            _serial.setDebugOutput(SERIAL_DEBUG_ENABLE);
        };
        void Debug(String component, String message, ...);
        unsigned int Step(){
            Debugger::_stepCount++;
            Debugger::Debug("step", "%d", Debugger::_stepCount);
            return Debugger::_stepCount;
        };
        void addHandler(void (*newHandler)(String message))
        {
            Debugger::_handlers.push_back(newHandler);
        };

    private:
        HardwareSerial _serial = Serial;
        static String lastComponent;
        static char lastBuff[DEBUG_SERIAL_BUFFER_SIZE];
        static int duplicates;
        static handlersList  _handlers;
        static unsigned int _stepCount;
};

String Debugger::lastComponent;
char Debugger::lastBuff[DEBUG_SERIAL_BUFFER_SIZE];
int Debugger::duplicates = 0;
handlersList Debugger::_handlers;
unsigned int Debugger::_stepCount;
#endif