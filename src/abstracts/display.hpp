#pragma once

#include "kern_inc.h"
#include "driver.hpp"

class AbstractDisplay : public sDOS_Abstract_Driver {
public:
    AbstractDisplay(Debugger *debugger, EventsManager *eventsManager) : sDOS_Abstract_Driver(debugger, eventsManager) {}

    virtual void writePixel(uint16_t x, uint16_t y, uint16_t value) = 0;

    virtual void writePixels(uint16_t *colors, uint32_t len, bool block = true, bool bigEndian = false) = 0;

    virtual void setCursor(int16_t x, int16_t y) = 0;

    virtual void beginRedraw() = 0;

    virtual void commitRedraw() = 0;

    virtual void setBacklight(unsigned int backlight) = 0;

    virtual void setEnabled(bool on) = 0;
};
