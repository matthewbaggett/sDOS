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

boolean sdos_is_wifi_active();
boolean sdos_is_bluetooth_active();

using namespace std;
using handlersList = std::list<void(*)(String message)>;


class Debugger
{
    public:
        Debugger();
        void Debug(String component, String message, ...);
        void addHandler(void (*newHandler)(String));

    private:
        HardwareSerial _serial = Serial;
        static String lastComponent;
        static char lastBuff[DEBUG_SERIAL_BUFFER_SIZE];
        static int duplicates;
        static handlersList  _handlers;
};

String Debugger::lastComponent;
char Debugger::lastBuff[DEBUG_SERIAL_BUFFER_SIZE];
int Debugger::duplicates = 0;
handlersList Debugger::_handlers;

Debugger::Debugger()
{
    _serial = Serial;
    _serial.begin(SERIAL_BAUD);
    _serial.setDebugOutput(SERIAL_DEBUG_ENABLE);
};

void Debugger::addHandler(void (*newHandler)(String message))
{
    Debugger::_handlers.push_back(newHandler);
};
