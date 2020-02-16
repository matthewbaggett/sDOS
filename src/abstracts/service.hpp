#ifndef ABSTRACT_sDOS_SERVICE_h
#define ABSTRACT_sDOS_SERVICE_h

#include "kern_inc.h"

class sDOS_Abstract_Service {
public:
    virtual void setup();

    virtual void loop();

    virtual bool isActive() { return true; };

    virtual String getName() { return "bad service"; };
};

#endif