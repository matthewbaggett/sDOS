#ifndef ADAFRUIT_ST7735_SHIM_HPP
#define ADAFRUIT_ST7735_SHIM_HPP

#include <Adafruit_GFX.h>  // Core graphics library
#include <Adafruit_ST77xx.h>
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_ST7789.cpp>

class sDOS_Adafruit_ST7789_Shim : public Adafruit_ST7789 {
public:
    sDOS_Adafruit_ST7789_Shim(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst = -1)
        : Adafruit_ST7789(cs, dc, mosi, sclk, rst) {};

    sDOS_Adafruit_ST7789_Shim(int8_t cs, int8_t dc, int8_t rst)
        : Adafruit_ST7789(cs, dc, rst) {};
#if !defined(ESP8266)

    sDOS_Adafruit_ST7789_Shim(SPIClass *spiClass, int8_t cs, int8_t dc, int8_t rst)
        : Adafruit_ST7789(spiClass, cs, dc, rst) {};
#endif // end !ESP8266

    void setColumnOffset(int offset) {
        _colstart = offset;
    };

    void setRowOffset(int offset) {
        _rowstart = offset;
    }

    void init(uint16_t width, uint16_t height, uint8_t mode, uint8_t colstart = 0, uint8_t rowstart = 0) {
        spiMode = mode;
        commonInit(NULL);

        _width = width;
        _height = height;
        _colstart = colstart;
        _rowstart = rowstart;

        displayInit(generic_st7789);
        setRotation(0);
    }
};

#endif