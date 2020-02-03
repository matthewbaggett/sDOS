#ifndef ABSTRACT_SDOS_SERVICE_h
#define ABSTRACT_SDOS_SERVICE_h
#include "includes.h"

class sDOS_Abstract_Service{
    public: 
    virtual void setup();
    virtual void loop();
};
#endif