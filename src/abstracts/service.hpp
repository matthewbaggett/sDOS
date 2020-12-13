#pragma once
#include "kern_inc.h"

class sDOS_Abstract_Service {
public:
    virtual void setup();

    virtual void loop();

    virtual bool isActive() {
        return true;
    };

    virtual String getName() {
        return "bad service";
    };
};
