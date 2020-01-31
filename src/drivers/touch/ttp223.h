#include "includes.h"

#pragma message("Included TTP223")

class TTP223
{
public:
    TTP223(EventsManager &eventsManager);
    static void interrupt();
    void loop();

private:
    static boolean interruptTriggered;
    EventsManager _events;
};

TTP223::TTP223(EventsManager &eventsManager) : _events(eventsManager)
{
    _events.trigger("ttp223_setup");
// If we have a power pin to drive this sensor, enable it
#ifdef PIN_POWER_TTP223
    pinMode(PIN_POWER_TTP223, OUTPUT);
    digitalWrite(PIN_POWER_TTP223, HIGH);
#endif
    pinMode(PIN_INTERRUPT_TTP223, INPUT);
    attachInterrupt(PIN_INTERRUPT_TTP223, interrupt, RISING);
    _events.trigger("ttp223_setup2");
};

void TTP223::interrupt()
{
    interruptTriggered = true;
};

void TTP223::loop()
{
    if (interruptTriggered)
    {
        _events.trigger("ttp223");
    }
};
