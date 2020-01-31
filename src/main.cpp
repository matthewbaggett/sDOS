#include "sdos.h"

sDOS instance;

void setup()
{
    instance.Run();
}

void loop()
{
    Serial.println("Arsche");

    instance.Loop();
}