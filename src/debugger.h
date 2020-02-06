#include "includes.h"
#define COL_YELLOW "\033[1;33m"
#define COL_GREEN "\033[1;32m"
#define COL_RED   "\033[1;31m"
#define COL_RESET "\033[0m"

class Debugger
{
public:
    Debugger()
    {
        _serial = Serial;
        _serial.begin(SERIAL_BAUD);
        _serial.setDebugOutput(SERIAL_DEBUG_ENABLE);
    }

    //void Debug(String component, String message);
    void Debug(String component, String message, ...);

private:
    HardwareSerial _serial = Serial;
    static String lastComponent;
    static char lastBuff[128];
    static int duplicates;
};

String Debugger::lastComponent;
char Debugger::lastBuff[128];
int Debugger::duplicates = 0;

void Debugger::Debug(String component, String format, ...)
{
    char buff[128];
    va_list args;
    va_start(args, format.c_str());
    vsprintf(buff, format.c_str(), args);

    /*Serial.printf(
        "\n  Component     : %s\n  LastComponent : %s\n  Buff          : %s\n  LastBuff      : %s\n",
        component,
        Debugger::lastComponent,
        buff,
        Debugger::lastBuff
    );*/

    if (Debugger::lastComponent.equals(component) && strcmp(buff, Debugger::lastBuff) == 0)
    {
        Debugger::duplicates++;
        _serial.printf("\t\t(repeated %d times)\r", Debugger::duplicates);
        return;
    }

    component.toUpperCase();

    _serial.printf(
        "%s[%s%.7-7s%s %s%dMhz%s %s%s%s] %s\n", 
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
        buff
    );
    memcpy(Debugger::lastBuff, buff, sizeof(buff));
    Debugger::lastComponent = component;
    Debugger::duplicates = 0;
    return;
};

