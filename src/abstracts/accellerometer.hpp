#pragma once
#include "kern_inc.h"
#include "driver.hpp"

class AbstractAccellerometer : public sDOS_Abstract_Driver {
public:
    AbstractAccellerometer(Debugger *debugger, EventsManager *eventsManager) : sDOS_Abstract_Driver(debugger, eventsManager) {
    }

};