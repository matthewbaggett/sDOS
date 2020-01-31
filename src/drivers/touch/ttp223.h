#include "includes.h"

#pragma message("Included TTP223")

void interrupt_ttp223();

class TTP223
{
public:
    TTP223(EventsManager &eventsManager);
    void loop();
    void enable();
    void disable();

private:
    static void interrupt();
    static bool hasInterruptOccured();
    static bool interruptTriggered;
    EventsManager _events;
};

bool TTP223::interruptTriggered = false;

TTP223::TTP223(EventsManager &eventsManager) : _events(eventsManager)
{
// If we have a power pin to drive this sensor, enable it
#ifdef PIN_POWER_TTP223
#pragma message("TTP223's PIN_POWER_TTP223 is defined, so we must provide power to drive the TTP223 chip")
    pinMode(PIN_POWER_TTP223, OUTPUT);
    enable();
#endif
    pinMode(PIN_INTERRUPT_TTP223, INPUT);
    attachInterrupt(PIN_INTERRUPT_TTP223, TTP223::interrupt, RISING);
    _events.trigger("ttp223_ready");
};

void TTP223::interrupt()
{
    interruptTriggered = true;
};

bool TTP223::hasInterruptOccured()
{
    if (interruptTriggered)
    {
        interruptTriggered = false;
        return true;
    }
    return false;
}

void TTP223::loop()
{
    if (TTP223::hasInterruptOccured())
    {
        _events.trigger("ttp223");
    }
};

void TTP223::enable()
{
    _events.trigger("ttp223_enable");
#ifdef PIN_POWER_TTP223
    digitalWrite(PIN_POWER_TTP223, HIGH);
#endif
}

void TTP223::disable()
{
    _events.trigger("ttp223_disable");
#ifdef PIN_POWER_TTP223
    digitalWrite(PIN_POWER_TTP223, LOW);
#endif
}