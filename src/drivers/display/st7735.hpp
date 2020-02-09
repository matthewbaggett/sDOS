#include "kern_inc.h"
#include "abstracts/display.hpp"
#include <Adafruit_GFX.h>  // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
//#include "../lib/Adafruit/ST7735/Adafruit_ST7789.h" // Hardware-specific library for ST7789
#include <SPI.h>

class sDOS_DISPLAY_ST7735 : public AbstractDisplay
{
    public:
        sDOS_DISPLAY_ST7735(Debugger &debugger, EventsManager &eventsManager, sDOS_SPI *sdos_spi) 
            : _debugger(debugger), _eventsManager(eventsManager), _sdos_spi(sdos_spi) {};

        void setup() {
            setupBacklight();
            setupScreen();
            demo();
        };
        void setupBacklight(){
            _debugger.Debug(_component, "setup()");
            pinMode(ST77XX_BL, OUTPUT);
            ledcSetup(_pwmChannel, 5000, 8);
            ledcAttachPin(ST77XX_BL, _pwmChannel);
        };
        void setupScreen(){
            _tft.initR(0x99);
            #ifdef ST77XX_ROTATION
            _tft.setRotation(ST77XX_ROTATION);
            #endif       
        };
        void demo(){
            _tft.setFont();
            _tft.fillScreen(ST77XX_WHITE);
            _tft.setTextColor(ST77XX_WHITE);
            _tft.setTextSize(2);
        };

        void loop() {
            updateBacklight();
            //sDOS_DISPLAY_ST7735::_backlightBrightness = (sDOS_DISPLAY_ST7735::_backlightBrightness == 255) ? 0 : 255;
            _tft.setCursor(0+0, (DISPLAY_WIDTH/2)+0);
            _tft.setTextColor(ST77XX_RED);
            _tft.print("ST7789");
            _tft.setCursor(0+2, (DISPLAY_WIDTH/2)+2);
            _tft.setTextColor(ST77XX_GREEN);
            _tft.print("ST7789");
            _tft.setCursor(0+4, (DISPLAY_WIDTH/2)+4);
            _tft.setTextColor(ST77XX_BLUE);
            _tft.print("ST7789");
            
            _tft.enableDisplay(sDOS_DISPLAY_ST7735::_displayOn);
        };

        void updateBacklight(){
            ledcWrite(_pwmChannel, sDOS_DISPLAY_ST7735::_displayOn ? sDOS_DISPLAY_ST7735::_backlightBrightness : 0);
            //_debugger.Debug(_component, "Backlight %d%%", sDOS_DISPLAY_ST7735::_backlightBrightness);
        };

        String getName(){ return _component; };


    protected:
        String _component = "ST7735";
        Debugger _debugger;
        EventsManager _eventsManager;
        sDOS_SPI *_sdos_spi;
        static bool _displayOn;
        static unsigned int _backlightBrightness;   // 0-255 brightness
        Adafruit_ST7735 _tft = Adafruit_ST7735(ST77XX_CS, ST77XX_DC, SPI_MOSI, SPI_SCLK, ST77XX_RST);
        const uint16_t  _Display_Color_Black        = 0x0000;
        const uint16_t  _Display_Color_Blue         = 0x001F;
        const uint16_t  _Display_Color_Red          = 0xF800;
        const uint16_t  _Display_Color_Green        = 0x07E0;
        const uint16_t  _Display_Color_Cyan         = 0x07FF;
        const uint16_t  _Display_Color_Magenta      = 0xF81F;
        const uint16_t  _Display_Color_Yellow       = 0xFFE0;
        const uint16_t  _Display_Color_White        = 0xFFFF;
        int _pwmChannel = sDOS_DISPLAY_ST7735::getUnusedPWMChannel();
};

unsigned int sDOS_DISPLAY_ST7735::_backlightBrightness = 250;
bool sDOS_DISPLAY_ST7735::_displayOn = true;
