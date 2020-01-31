#include "sdos.h"

sDOS instance;

void setup()
{
    Serial.begin(SERIAL_BAUD);
    Serial.setDebugOutput(SERIAL_DEBUG_ENABLE);
    delay(200);
    instance.Run();
}

void loop()
{
   instance.Loop();
}