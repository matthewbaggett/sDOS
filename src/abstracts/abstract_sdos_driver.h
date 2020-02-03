#ifndef ABSTRACT_SDOS_DRIVER_H
#define ABSTRACT_SDOS_DRIVER_H
#include "includes.h"

class sDOS_Abstract_Driver{
    public: 
    virtual void setup();
    virtual void loop();
    virtual boolean isActive();
};
#endif