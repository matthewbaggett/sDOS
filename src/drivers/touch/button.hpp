#ifndef SDOS_DRIVERS_TOUCH_BUTTON_HPP
#define SDOS_DRIVERS_TOUCH_BUTTON_HPP

#include "kern_inc.h"
#include "abstracts/driver.hpp"

class sDOS_BUTTON : public sDOS_Abstract_Driver {
public:
    sDOS_BUTTON(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger),
                                                                    _eventsManager(eventsManager) {}

    void setup() override {
        _eventsManager.trigger("button_ready");
    };

    void loop() override {

    };

    String getName() {
        return _component;
    };

protected:
    static void interrupt() {
    };

    String _component = "button";
    Debugger _debugger;
    EventsManager _eventsManager;

};

#endif