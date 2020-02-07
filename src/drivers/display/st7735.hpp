#include "kern_inc.h"
#include "abstracts/display.hpp"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
//#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

class SDOS_DISPLAY_ST7735 : public AbstractDisplay
{
    public:
        SDOS_DISPLAY_ST7735(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger), _events(eventsManager) {}
    private:
        Debugger _debugger;
        EventsManager _eventsManager;
        Adafruit_ST7735 _tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
{
}

