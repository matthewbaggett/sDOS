#ifndef TTP223_H
#define TTP223_H
#include "kernel_inc.h"

class SDOS_TTP223 : public sDOS_Abstract_Driver
{
public:
    SDOS_TTP223(Debugger & debugger, EventsManager & eventsManager){
        _debugger = debugger;
        _events = eventsManager;
    };
    void setup();
    void loop();
    void enable();
    void disable();

private:
    static void interrupt();
    static bool hasInterruptOccured();
    static bool interruptTriggered;
    Debugger _debugger;
    EventsManager _events;
};

bool SDOS_TTP223::interruptTriggered = false;

void SDOS_TTP223::setup(){
#ifdef PIN_POWER_TTP223
// If we have a power pin to drive this sensor, enable it
    pinMode(PIN_POWER_TTP223, OUTPUT);
    enable();
#endif
    pinMode(PIN_INTERRUPT_TTP223, INPUT);
    attachInterrupt(PIN_INTERRUPT_TTP223, SDOS_TTP223::interrupt, RISING);
    gpio_wakeup_enable(PIN_INTERRUPT_TTP223, GPIO_INTR_HIGH_LEVEL);
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
        _events.trigger("TTP223_interrupt");
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
#endif