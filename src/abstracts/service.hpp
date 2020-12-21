#pragma once
#include "kern_inc.h"

class sDOS_Abstract_Service {
public:
    sDOS_Abstract_Service(Debugger * debugger, EventsManager * eventsManager) : _debugger(debugger), _eventsManager(eventsManager){
    }

    virtual void loop(){};

    virtual void setup(){};

    virtual bool isActive() {
        return true;
    };

    virtual String getName() {
        return "bad service";
    };

protected:
    Debugger * _debugger;
    EventsManager * _eventsManager;
};
