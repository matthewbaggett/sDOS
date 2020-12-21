#pragma once
#include "kern_inc.h"

class sDOS_Abstract_Driver {
public:
    sDOS_Abstract_Driver(Debugger * debugger, EventsManager * eventsManager) : _debugger(debugger), _eventsManager(eventsManager){
    }

    virtual void setup(){};

    virtual void loop(){};

    virtual bool isActive() {
        return true;
    }

    virtual String getName() = 0;

    static int getUnusedPWMChannel() {
        sDOS_Abstract_Driver::_allocatedPWMCount++;
        return sDOS_Abstract_Driver::_allocatedPWMCount;
    };

private:
    static int _allocatedPWMCount;
protected:
    Debugger * _debugger;
    EventsManager * _eventsManager;
};

int sDOS_Abstract_Driver::_allocatedPWMCount = -1;