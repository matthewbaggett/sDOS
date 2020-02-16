#include "kern_inc.h"
#include "abstracts/driver.hpp"

class sDOS_TTP223 : public sDOS_Abstract_Driver {
public:
    sDOS_TTP223(Debugger &debugger, EventsManager &eventsManager);

    void setup();

    void loop();

    void enable();

    void disable();

    String getName() { return _component; };

private:
    static void interrupt();

    static bool hasInterruptOccuredButtonDown();

    static bool hasInterruptOccuredButtonUp();

    static bool interruptTriggeredButtonDown;
    static bool interruptTriggeredButtonUp;
    static uint64_t interruptDownOccured;
    String _component = "ttp223";
    Debugger _debugger;
    EventsManager _events;
};

bool sDOS_TTP223::interruptTriggeredButtonDown = false;
bool sDOS_TTP223::interruptTriggeredButtonUp = false;
uint64_t sDOS_TTP223::interruptDownOccured = 0;

sDOS_TTP223::sDOS_TTP223(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger),
                                                                             _events(eventsManager) {}

void sDOS_TTP223::setup() {
#ifdef PIN_POWER_TTP223
    // If we have a power pin to drive this sensor, enable it
    pinMode(PIN_POWER_TTP223, OUTPUT);
    enable();
#endif
    pinMode(PIN_INTERRUPT_TTP223, INPUT);
    attachInterrupt(PIN_INTERRUPT_TTP223, sDOS_TTP223::interrupt, CHANGE);
    gpio_wakeup_enable(PIN_INTERRUPT_TTP223, GPIO_INTR_HIGH_LEVEL);
    _events.trigger("TTP223_ready");
};

void sDOS_TTP223::interrupt() {
    if (digitalRead(PIN_INTERRUPT_TTP223)) {
        interruptTriggeredButtonDown = true;
    } else {
        interruptTriggeredButtonUp = true;
    }
};

bool sDOS_TTP223::hasInterruptOccuredButtonDown() {
    if (interruptTriggeredButtonDown) {
        interruptTriggeredButtonDown = false;
        interruptDownOccured = micros();
        return true;
    }
    return false;
}

bool sDOS_TTP223::hasInterruptOccuredButtonUp() {
    if (interruptTriggeredButtonUp) {
        interruptDownOccured = micros() - interruptDownOccured;
        interruptTriggeredButtonUp = false;
        return true;
    }
    return false;
}

void sDOS_TTP223::loop() {
    if (sDOS_TTP223::hasInterruptOccuredButtonDown()) {
        _events.trigger("TTP223_down");
    }
    if (sDOS_TTP223::hasInterruptOccuredButtonUp()) {
        unsigned int heldDownMs = interruptDownOccured / 1000;
        _events.trigger("TTP223_up", heldDownMs);
    }
};

void sDOS_TTP223::enable() {
    _events.trigger("TTP223_enable");
#ifdef PIN_POWER_TTP223
    digitalWrite(PIN_POWER_TTP223, HIGH);
#endif
}

void sDOS_TTP223::disable() {
    _events.trigger("TTP223_disable");
#ifdef PIN_POWER_TTP223
    digitalWrite(PIN_POWER_TTP223, LOW);
#endif
}