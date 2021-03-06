#pragma once
#include "kern_inc.h"
#include "abstracts/driver.hpp"

class sDOS_BUTTON : public sDOS_Abstract_Driver {
public:
    sDOS_BUTTON(Debugger *debugger, EventsManager *eventsManager) : sDOS_Abstract_Driver(debugger, eventsManager) {
        _eventsManager->trigger("button_ready");
        //debugger->Debug(_component, "Construct");
    }

    void loop() override {};

    String getName() {
        return _component;
    };

protected:
    static void interrupt() {
    };

    String _component = "button";

};

