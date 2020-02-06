#ifndef ABSTRACT_SDOS_DRIVER_H
#define ABSTRACT_SDOS_DRIVER_H
#include "kern_inc.h"

class sDOS_Abstract_Driver{
    public: 
    virtual void setup();
    virtual void loop();
    virtual boolean isActive() { return true; }
};
#endif