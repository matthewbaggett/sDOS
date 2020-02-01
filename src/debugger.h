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
    static char *lastBuff;
    static int duplicates;
};

String Debugger::lastComponent;
char * Debugger::lastBuff = "";
int Debugger::duplicates = 0;

void Debugger::Debug(String component, String format, ...)
{
    char buff[128];
    va_list args;
    va_start(args, format.c_str());
    vsprintf(buff, format.c_str(), args);

    //_serial.printf("[%s] %s\n", component.c_str(), buff);
    //return;

    if (!(component ==  Debugger::lastComponent && buff ==  Debugger::lastBuff))
    {
        if ( Debugger::duplicates > 0)
        {
            _serial.println();
        }
        _serial.printf("[%s] %s\n", component.c_str(), buff);
         Debugger::lastComponent = component;
         Debugger::lastBuff = buff;
         Debugger::duplicates = 0;
    }
    else
    {
         Debugger::duplicates++;
        _serial.printf("\t\t(repeated %d times)\r",  Debugger::duplicates);
    }
};

//void Debugger::Debug(String component, String message){
//    _serial.printf("[%s] %s\n", component.c_str(), message.c_str());
//};
