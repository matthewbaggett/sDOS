#include "kern_inc.h"
#include "abstracts/display.hpp"

#include <Adafruit_GFX.h>  // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

class SDOS_DISPLAY_ST7789 : public AbstractDisplay
{
    public:
        SDOS_DISPLAY_ST7789(Debugger &debugger, EventsManager &eventsManager, SDOS_SPI *sdos_spi) 
            : _debugger(debugger), _eventsManager(eventsManager), _sdos_spi(sdos_spi) {};

        void setup() {
            _debugger.Debug(_component, "setup()");
            setupBacklight();
            setupScreen();
            setupBuffer();
            demo();
        };
        void setupBacklight(){
            pinMode(ST77XX_BL, OUTPUT);
            ledcSetup(_pwmChannel, 5000, 8);
            ledcAttachPin(ST77XX_BL, _pwmChannel);
        };
        void setupScreen(){
            _tft.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
            #ifdef ST77XX_ROTATION
            _tft.setRotation(ST77XX_ROTATION);
            #endif
        };
        void setupBuffer(){
            _tft.setFont();
            _tft.fillScreen(ST77XX_BLACK);
            _tft.setTextColor(ST77XX_WHITE);
            _tft.setTextSize(3);
        };
        void demo(){
            //SDOS_DISPLAY_ST7789::_backlightBrightness = (SDOS_DISPLAY_ST7789::_backlightBrightness == 255) ? 0 : 255;
            _tft.setCursor(DISPLAY_WIDTH/2,DISPLAY_HEIGHT/2);
            _tft.setTextColor(ST77XX_RED);
            _tft.print("ST7789");
            _tft.setCursor((DISPLAY_WIDTH/2)+1,(DISPLAY_HEIGHT/2)+1);
            _tft.setTextColor(ST77XX_GREEN);
            _tft.print("ST7789");
            _tft.setCursor((DISPLAY_WIDTH/2)+2,(DISPLAY_HEIGHT/2)+2);
            _tft.setTextColor(ST77XX_BLUE);
            _tft.print("ST7789");
            
            _tft.enableDisplay(SDOS_DISPLAY_ST7789::_displayOn);
        }

        void loop() {
            updateBacklight();
            demo();

        };

        void updateBacklight(){
            ledcWrite(_pwmChannel, SDOS_DISPLAY_ST7789::_displayOn ? SDOS_DISPLAY_ST7789::_backlightBrightness : 0);
            //_debugger.Debug(_component, "Backlight %d%%", SDOS_DISPLAY_ST7789::_backlightBrightness);
        };

        String getName(){ return _component; };


    protected:
        String _component = "ST7789";
        Debugger _debugger;
        EventsManager _eventsManager;
        SDOS_SPI *_sdos_spi;
        static boolean _displayOn;
        static unsigned int _backlightBrightness;   // 0-255 brightness
        Adafruit_ST7789 _tft = Adafruit_ST7789(ST77XX_CS, ST77XX_DC, SPI_MOSI, SPI_SCLK, ST77XX_RST);
        const uint16_t  _Display_Color_Black        = 0x0000;
        const uint16_t  _Display_Color_Blue         = 0x001F;
        const uint16_t  _Display_Color_Red          = 0xF800;
        const uint16_t  _Display_Color_Green        = 0x07E0;
        const uint16_t  _Display_Color_Cyan         = 0x07FF;
        const uint16_t  _Display_Color_Magenta      = 0xF81F;
        const uint16_t  _Display_Color_Yellow       = 0xFFE0;
        const uint16_t  _Display_Color_White        = 0xFFFF;
        int _pwmChannel = SDOS_DISPLAY_ST7789::getUnusedPWMChannel();
};

unsigned int SDOS_DISPLAY_ST7789::_backlightBrightness = 255;
boolean SDOS_DISPLAY_ST7789::_displayOn = true;
