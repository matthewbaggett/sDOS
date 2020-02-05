#ifndef ABSTRACT_SDOS_SERVICE_h
#define ABSTRACT_SDOS_SERVICE_h
#include "../kernel_inc.h"

class sDOS_Abstract_Service{
    public: 
    virtual void setup();
    virtual void loop();
    virtual boolean isActive() { return true; }
};
#endif