#include "kern_inc.h"
#include "abstracts/driver.hpp"
#include <SPI.h>

class sDOS_SPI : public sDOS_Abstract_Driver {
public:
    sDOS_SPI(Debugger &debugger, EventsManager &eventsManager)
            : _debugger(debugger),
              _eventsManager(eventsManager) {};

    void setup() override {};

    void loop() override {};

    String getName() override { return _component; };

private:
    String _component = "SPI";
    Debugger _debugger;
    EventsManager _eventsManager;
    SPIClass *_spi;
};
