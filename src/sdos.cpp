#include "kern.hpp"

sDOS instance;

void setup() {
    Serial.begin(SERIAL_BAUD);
    Serial.setDebugOutput(SERIAL_DEBUG_ENABLE);
    delay(300);
    Serial.println("Alive?");
    instance.Setup();
}

void loop() {
    instance.Loop();
}