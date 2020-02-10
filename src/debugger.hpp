#include "kern_inc.h"
#include <algorithm>
#include <iostream>
#include <list>

#define COL_RESET  "\033[0m"
#define COL_RED    "\033[1;31m"
#define COL_GREEN  "\033[1;32m"
#define COL_YELLOW "\033[1;33m"
#define COL_BLUE  "\033[1;34m"
#define COL_PINK  "\033[1;35m"
#define DEBUG_SERIAL_BUFFER_SIZE 255

bool sdos_is_wifi_active();
bool sdos_is_bluetooth_active();

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