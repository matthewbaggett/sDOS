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
};

void Debugger::Debug(String component, String format, ...)
{
    Serial.print("Debugging debug: ");
    Serial.println(format.c_str());

    char buff[128];
    va_list args;
    va_start(args, format.c_str());
    vsprintf(buff, format.c_str(), args);

    _serial.printf("[%s] %s\n", component.c_str(), buff);
};

//void Debugger::Debug(String component, String message){
//    _serial.printf("[%s] %s\n", component.c_str(), message.c_str());
//};
