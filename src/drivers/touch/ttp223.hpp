#include "kern_inc.h"
#include "abstracts/driver.hpp"

class SDOS_TTP223 : public sDOS_Abstract_Driver
{
public:
    SDOS_TTP223(Debugger &debugger, EventsManager &eventsManager);
    void setup();
    void loop();
    void enable();
    void disable();

private:
    static void interrupt();
    static bool hasInterruptOccuredButtonDown();
    static bool hasInterruptOccuredButtonUp();
    static bool interruptTriggeredButtonDown;
    static bool interruptTriggeredButtonUp;
    Debugger _debugger;
    EventsManager _events;
};

bool SDOS_TTP223::interruptTriggeredButtonDown = false;
bool SDOS_TTP223::interruptTriggeredButtonUp = false;

SDOS_TTP223::SDOS_TTP223(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger), _events(eventsManager)
{}

void SDOS_TTP223::setup(){
#ifdef PIN_POWER_TTP223
// If we have a power pin to drive this sensor, enable it
    pinMode(PIN_POWER_TTP223, OUTPUT);
    enable();
#endif
    pinMode(PIN_INTERRUPT_TTP223, INPUT);
    attachInterrupt(PIN_INTERRUPT_TTP223, SDOS_TTP223::interrupt, CHANGE);
    gpio_wakeup_enable(PIN_INTERRUPT_TTP223, GPIO_INTR_HIGH_LEVEL);
    _events.trigger("TTP223_ready");
};

void SDOS_TTP223::interrupt()
{
    if(digitalRead(PIN_INTERRUPT_TTP223)){
        interruptTriggeredButtonDown = true;
    }else{
        interruptTriggeredButtonUp = true;
    }
};

bool SDOS_TTP223::hasInterruptOccuredButtonDown()
{
    if (interruptTriggeredButtonDown)
    {
        interruptTriggeredButtonDown = false;
        return true;
    }
    return false;
}

bool SDOS_TTP223::hasInterruptOccuredButtonUp()
{
    if (interruptTriggeredButtonUp)
    {
        interruptTriggeredButtonUp = false;
        return true;
    }
    return false;
}

void SDOS_TTP223::loop()
{
    if (SDOS_TTP223::hasInterruptOccuredButtonDown())
    {
        _events.trigger("TTP223_down");
    }
    if (SDOS_TTP223::hasInterruptOccuredButtonUp())
    {
        _events.trigger("TTP223_up");
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