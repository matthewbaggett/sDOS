#include "kern_inc.h"
#include "abstracts/driver.hpp"

class sDOS_TTP223 : public sDOS_Abstract_Driver {
public:
    sDOS_TTP223(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger),
                                                                    _events(eventsManager) {};

    void setup() override {
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

    void loop() override {
        if (sDOS_TTP223::hasInterruptOccuredButtonDown()) {
            _events.trigger("TTP223_down");
        }
        if (sDOS_TTP223::hasInterruptOccuredButtonUp()) {
            unsigned int heldDownMs = _interruptDownOccurred / 1000;
            _events.trigger("TTP223_up", heldDownMs);
        }
    };

    void enable() {
        _events.trigger("TTP223_enable");
#ifdef PIN_POWER_TTP223
        digitalWrite(PIN_POWER_TTP223, HIGH);
#endif
    };

    void disable() {
        _events.trigger("TTP223_disable");
#ifdef PIN_POWER_TTP223
        digitalWrite(PIN_POWER_TTP223, LOW);
#endif
    };

    String getName() override { return _component; };

private:
    static void interrupt() {
        if (digitalRead(PIN_INTERRUPT_TTP223)) {
            _interruptTriggeredButtonDown = true;
        } else {
            _interruptTriggeredButtonUp = true;
        }
    };

    static bool hasInterruptOccuredButtonDown() {
        if (_interruptTriggeredButtonDown) {
            _interruptTriggeredButtonDown = false;
            _interruptDownOccurred = micros();
            return true;
        }
        return false;
    };

    static bool hasInterruptOccuredButtonUp() {
        if (_interruptTriggeredButtonUp) {
            _interruptDownOccurred = micros() - _interruptDownOccurred;
            _interruptTriggeredButtonUp = false;
            return true;
        }
        return false;
    };

    static bool _interruptTriggeredButtonDown;
    static bool _interruptTriggeredButtonUp;
    static uint64_t _interruptDownOccurred;
    String _component = "ttp223";
    Debugger _debugger;
    EventsManager _events;
};

bool sDOS_TTP223::_interruptTriggeredButtonDown = false;
bool sDOS_TTP223::_interruptTriggeredButtonUp = false;
uint64_t sDOS_TTP223::_interruptDownOccurred = 0;
