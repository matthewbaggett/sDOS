#include "includes.h"

class Debugger
{
public:
    Debugger()
    {
        _serial = Serial;
        _serial.begin(SERIAL_BAUD);
        _serial.setDebugOutput(SERIAL_DEBUG_ENABLE);
        delay(300);
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

    _serial.printf("%s[%s] %s\n", Debugger::duplicates > 0 ? "\n":"", component.c_str(), buff);
    memcpy(Debugger::lastBuff, buff, sizeof(buff));
    Debugger::lastComponent = component;
    Debugger::duplicates = 0;
    return;
};

