#include "kern_inc.h"
#include "abstracts/driver.hpp"

class sDOS_BUTTON : public sDOS_Abstract_Driver {
public:
    sDOS_BUTTON(Debugger &debugger, EventsManager &eventsManager);

    void setup();

    void loop();

    String getName() { return _component; };

private:
    static void interrupt();

    String _component = "button";
    Debugger _debugger;
    EventsManager _events;
#ifdef BUTTON_0
    static bool _button_0_state;
    static bool _button_0_interrupt_fired;
#endif
#ifdef BUTTON_1
    static bool _button_1_state;
    static bool _button_1_interrupt_fired;
#endif
};

#ifdef BUTTON_0
bool sDOS_BUTTON::_button_0_state = false;
bool sDOS_BUTTON::_button_0_interrupt_fired = false;
#endif
#ifdef BUTTON_1
bool sDOS_BUTTON::_button_1_state = false;
bool sDOS_BUTTON::_button_1_interrupt_fired = false;
#endif


sDOS_BUTTON::sDOS_BUTTON(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger),
                                                                             _events(eventsManager) {}

void sDOS_BUTTON::setup() {
#ifdef BUTTON_O
    pinMode(BUTTON_0, INPUT);
    attachInterrupt(BUTTON_0, sDOS_BUTTON::interrupt, CHANGE);
    gpio_wakeup_enable(BUTTON_0, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(BUTTON_0, GPIO_INTR_HIGH_LEVEL);
#endif
#ifdef BUTTON_1
    pinMode(BUTTON_1, INPUT);
    attachInterrupt(BUTTON_1, sDOS_BUTTON::interrupt, CHANGE);
    gpio_wakeup_enable(BUTTON_0, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(BUTTON_1, GPIO_INTR_HIGH_LEVEL);
#endif

    _events.trigger("button_ready");
};

void sDOS_BUTTON::interrupt() {
#ifdef BUTTON_0
    if(digitalRead(BUTTON_0) != sDOS_BUTTON::_button_0_state){
        sDOS_BUTTON::_button_0_state = digitalRead(BUTTON_0);
        sDOS_BUTTON::_button_0_interrupt_fired = true;
    }
#endif
#ifdef BUTTON_1
    if(digitalRead(BUTTON_1) != sDOS_BUTTON::_button_1_state){
        sDOS_BUTTON::_button_1_state = digitalRead(BUTTON_0);
        sDOS_BUTTON::_button_1_interrupt_fired = true;
    }
#endif
};

void sDOS_BUTTON::loop() {
#ifdef BUTTON_0
    if(sDOS_BUTTON::_button_0_interrupt_fired){
        if(sDOS_BUTTON::_button_0_state){
            _events.trigger("button_0", F("up"));
        }else{
            _events.trigger("button_0", F("down"));
        }
        sDOS_BUTTON::_button_0_interrupt_fired = false;
    }
#endif
#ifdef BUTTON_1
    if(sDOS_BUTTON::_button_1_interrupt_fired){
        if(sDOS_BUTTON::_button_1_state){
            _events.trigger("button_1", F("up"));
        }else{
            _events.trigger("button_1", F("down"));
        }
        sDOS_BUTTON::_button_1_interrupt_fired = false;

    }
#endif
};
