#pragma once

#include "kern_inc.h"

class sDOS_Abstract_Driver {
public:
    virtual void setup() = 0;

    virtual void loop() = 0;

    virtual bool isActive() {
        return true;
    }

    virtual String getName() {
        return "bad driver";
    };

    static int getUnusedPWMChannel() {
        sDOS_Abstract_Driver::_allocatedPWMCount++;
        return sDOS_Abstract_Driver::_allocatedPWMCount;
    };

private:
    static int _allocatedPWMCount;
};

int sDOS_Abstract_Driver::_allocatedPWMCount = -1;