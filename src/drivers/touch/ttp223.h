#include "includes.h"

#pragma message("Included SDOS_TTP223")

class SDOS_TTP223
{
public:
    SDOS_TTP223(EventsManager &eventsManager);
    void setup();
    void loop();
    void enable();
    void disable();

private:
    static void interrupt();
    static bool hasInterruptOccured();
    static bool interruptTriggered;
    EventsManager _events;
};

bool SDOS_TTP223::interruptTriggered = false;

SDOS_TTP223::SDOS_TTP223(EventsManager &eventsManager) : _events(eventsManager)
{}

void SDOS_TTP223::setup(){
// If we have a power pin to drive this sensor, enable it
#ifdef PIN_POWER_TTP223
#pragma message("TTP223's PIN_POWER_TTP223 is defined, so we must provide power to drive the TTP223 chip")
    pinMode(PIN_POWER_TTP223, OUTPUT);
    enable();
#endif
    pinMode(PIN_INTERRUPT_TTP223, INPUT);
    attachInterrupt(PIN_INTERRUPT_TTP223, SDOS_TTP223::interrupt, RISING);
    _events.trigger("TTP223_ready");
};

void SDOS_TTP223::interrupt()
{
    interruptTriggered = true;
};

bool SDOS_TTP223::hasInterruptOccured()
{
    if (interruptTriggered)
    {
        interruptTriggered = false;
        return true;
    }
    return false;
}

void SDOS_TTP223::loop()
{
    if (SDOS_TTP223::hasInterruptOccured())
    {
        _events.trigger("TTP223");
    }
};

void SDOS_TTP223::enable()
{
    _events.trigger("TTP223_enable");
#ifdef PIN_POWER_TTP223
    digitalWrite(PIN_POWER_TTP223, HIGH);
#endif
}

void SDOS_TTP223::disable()
{
    _events.trigger("TTP223_disable");
#ifdef PIN_POWER_TTP223
    digitalWrite(PIN_POWER_TTP223, LOW);
#endif
}