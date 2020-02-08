#include "kern_inc.h"
#include "abstracts/driver.hpp"

class SDOS_BUTTON : public sDOS_Abstract_Driver
{
public:
    SDOS_BUTTON(Debugger &debugger, EventsManager &eventsManager);
    void setup();
    void loop();
    String getName(){ return _component; };

private:
    static void interrupt();
    String _component = "button";
    Debugger _debugger;
    EventsManager _events;
    #ifdef BUTTON_0
    static boolean _button_0_state;
    static boolean _button_0_interrupt_fired;
    #endif
    #ifdef BUTTON_1
    static boolean _button_1_state;
    static boolean _button_1_interrupt_fired;
    #endif
};

#ifdef BUTTON_0
boolean SDOS_BUTTON::_button_0_state = false;
boolean SDOS_BUTTON::_button_0_interrupt_fired = false;
#endif
#ifdef BUTTON_1
boolean SDOS_BUTTON::_button_1_state = false;
boolean SDOS_BUTTON::_button_1_interrupt_fired = false;
#endif


SDOS_BUTTON::SDOS_BUTTON(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger), _events(eventsManager)
{}

void SDOS_BUTTON::setup(){
    #ifdef BUTTON_O
    pinMode(BUTTON_0, INPUT);
    attachInterrupt(BUTTON_0, SDOS_BUTTON::interrupt, CHANGE);
    gpio_wakeup_enable(BUTTON_0, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(BUTTON_0, GPIO_INTR_HIGH_LEVEL);
    #endif
    #ifdef BUTTON_1
    pinMode(BUTTON_1, INPUT);
    attachInterrupt(BUTTON_1, SDOS_BUTTON::interrupt, CHANGE);
    gpio_wakeup_enable(BUTTON_0, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(BUTTON_1, GPIO_INTR_HIGH_LEVEL);
    #endif
    
    _events.trigger("button_ready");
};

void SDOS_BUTTON::interrupt()
{
    #ifdef BUTTON_0
    if(digitalRead(BUTTON_0) != SDOS_BUTTON::_button_0_state){
        SDOS_BUTTON::_button_0_state = digitalRead(BUTTON_0);
        SDOS_BUTTON::_button_0_interrupt_fired = true;
    }
    #endif
    #ifdef BUTTON_1
    if(digitalRead(BUTTON_1) != SDOS_BUTTON::_button_1_state){
        SDOS_BUTTON::_button_1_state = digitalRead(BUTTON_0);
        SDOS_BUTTON::_button_1_interrupt_fired = true;
    }
    #endif
};

void SDOS_BUTTON::loop()
{
    #ifdef BUTTON_0
    if(SDOS_BUTTON::_button_0_interrupt_fired){
        if(SDOS_BUTTON::_button_0_state){
            _events.trigger("button_0", "up");
        }else{
            _events.trigger("button_0", "down");
        }
        SDOS_BUTTON::_button_0_interrupt_fired = false;
    }
    #endif
    #ifdef BUTTON_1
    if(SDOS_BUTTON::_button_1_interrupt_fired){
        if(SDOS_BUTTON::_button_1_state){
            _events.trigger("button_1", "up");
        }else{
            _events.trigger("button_1", "down");
        }
        SDOS_BUTTON::_button_1_interrupt_fired = false;

    }
    #endif
};
