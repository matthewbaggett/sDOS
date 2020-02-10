#ifndef ABSTRACT_DISPLAY_H 
#define ABSTRACT_DISPLAY_H
#include "kern_inc.h"
#include "driver.hpp"

class AbstractDisplay : public sDOS_Abstract_Driver
{
    public:
        virtual void writePixel(uint16_t x, uint16_t y, uint16_t value) = 0;
        virtual void beginRedraw() = 0;
        virtual void commitRedraw() = 0;
};
#endif