#pragma once

#include "kern_inc.h"
#include "driver.hpp"

class AbstractRTC : public sDOS_Abstract_Driver {
public:
    AbstractRTC(Debugger *debugger, EventsManager *eventsManager) : sDOS_Abstract_Driver(debugger, eventsManager) {
    }

    virtual void setTime(DateTime &newTime) = 0;

    virtual void setAlarmInMinutes(int minutes);

    virtual void setAlarmInSeconds(int seconds);

    virtual DateTime getTime();

    virtual void loop() override {
        sDOS_Abstract_Driver::loop();
    }
    virtual void setup() override{
        sDOS_Abstract_Driver::setup();
    }
};
