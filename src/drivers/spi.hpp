#include "kern_inc.h"
#include "abstracts/driver.hpp"
#include <SPI.h>

class SDOS_SPI : public sDOS_Abstract_Driver
{
    public:
        SDOS_SPI(Debugger &debugger, EventsManager &eventsManager);
        void setup();
        void loop();
        String getName(){ return _component; };
        
    private:
        String _component = "SPI";
        Debugger _debugger;
        EventsManager _eventsManager;
        SPIClass * _spi;
};

SDOS_SPI::SDOS_SPI(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger), _eventsManager(eventsManager){
    
};

void SDOS_SPI::setup(){
    //_spi = new SPIClass(VSPI);
    //_spi->begin(SPI_SCLK, SPI_MISO, SPI_MOSI, SPI_SS);
    //pinMode(SPI_SS, OUTPUT);
};

void SDOS_SPI::loop(){
    
}