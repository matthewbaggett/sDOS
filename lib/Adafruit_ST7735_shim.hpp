#ifndef ADAFRUIT_ST7735_SHIM_HPP
#define ADAFRUIT_ST7735_SHIM_HPP
#include <Adafruit_GFX.h>  // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
class sDOS_Adafruit_ST7735_Shim : public Adafruit_ST7735 {
    public:
        sDOS_Adafruit_ST7735_Shim(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk,
                  int8_t rst = -1);
        sDOS_Adafruit_ST7735_Shim(int8_t cs, int8_t dc, int8_t rst);
        #if !defined(ESP8266)
        sDOS_Adafruit_ST7735_Shim(SPIClass *spiClass, int8_t cs, int8_t dc, int8_t rst);
        #endif // end !ESP8266
        void setColumnOffset(int offset
        void setColumnOffset(int offset){
            _colstart = offset;
        };
        void setRowOffset(int offset){
            _rowstart = offset;
        }
};
#endif