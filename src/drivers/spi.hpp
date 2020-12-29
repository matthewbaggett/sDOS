#pragma once
#include "kern_inc.h"
#include "abstracts/driver.hpp"
#include <SPI.h>

class sDOS_SPI : public sDOS_Abstract_Driver {
public:
    sDOS_SPI(Debugger *debugger, EventsManager *eventsManager) : sDOS_Abstract_Driver(debugger, eventsManager) {
        //debugger->Debug(_component, "Construct");
    };

    void setup() override {};

    void loop() override {};

    String getName() {
        return _component;
    };

private:
    String _component = "SPI";
    SPIClass *_spi;
};
