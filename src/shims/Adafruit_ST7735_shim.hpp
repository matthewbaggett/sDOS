#pragma once

#include <Adafruit_GFX.h>  // Core graphics library
#include <Adafruit_ST77xx.h>
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7735.cpp> // Hardware-specific library for ST7735

static const uint8_t PROGMEM
tft160x80_1[] = {                       // 7735R init, part 1 (red or green tab)
    15,                             // 15 commands in list:
    ST77XX_SWRESET, ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
    150,                          //     150 ms delay
    ST77XX_SLPOUT, ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
    255,                          //     500 ms delay
    ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
    0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
    0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
    0x01, 0x2C, 0x2D,             //     Dot inversion mode
    0x01, 0x2C, 0x2D,             //     Line inversion mode
    ST7735_INVCTR, 1,              //  6: Display inversion ctrl, 1 arg:
    0x07,                         //     No inversion
    ST7735_PWCTR1, 3,              //  7: Power control, 3 args, no delay:
    0xA2,
    0x02,                         //     -4.6V
    0x84,                         //     AUTO mode
    ST7735_PWCTR2, 1,              //  8: Power control, 1 arg, no delay:
    0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    ST7735_PWCTR3, 2,              //  9: Power control, 2 args, no delay:
    0x0A,                         //     Opamp current small
    0x00,                         //     Boost frequency
    ST7735_PWCTR4, 2,              // 10: Power control, 2 args, no delay:
    0x8A,                         //     BCLK/2,
    0x2A,                         //     opamp current small & medium low
    ST7735_PWCTR5, 2,              // 11: Power control, 2 args, no delay:
    0x8A, 0xEE,
    ST7735_VMCTR1, 1,              // 12: Power control, 1 arg, no delay:
    0x0E,
    ST77XX_INVON, 0,              // 13: Don't invert display, no args
    ST77XX_MADCTL, 1,              // 14: Mem access ctl (directions), 1 arg:
    0xC8,                         //     row/col addr, bottom-top refresh
    ST77XX_COLMOD, 1,              // 15: set color mode, 1 arg, no delay:
    0x05
},                       //     16-bit color

tft160x80_2[] = {            // 7735R init, part 2 (mini 160x80)
    2,                              //  2 commands in list:
    ST77XX_CASET, 4,              //  1: Column addr set, 4 args, no delay:
    0x00, 0x00,                   //     XSTART = 0
    0x00, 0x4F,                   //     XEND = 79
    ST77XX_RASET, 4,              //  2: Row addr set, 4 args, no delay:
    0x00, 0x00,                   //     XSTART = 0
    0x00, 0x9F
},                 //     XEND = 159

tft160x80_3[] = {                       // 7735R init, part 3 (red or green tab)
    4,                              //  4 commands in list:
    ST7735_GMCTRP1, 16,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
    0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
    0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
    0x29, 0x25, 0x2B, 0x39,
    0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
    0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
    0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
    0x2E, 0x2E, 0x37, 0x3F,
    0x00, 0x00, 0x02, 0x10,
    ST77XX_NORON, ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
    10,                           //     10 ms delay
    ST77XX_DISPON, ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
    100
};;

class sDOS_Adafruit_ST7735_Shim : public Adafruit_ST7735 {
public:
    sDOS_Adafruit_ST7735_Shim(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst = -1) : Adafruit_ST7735(cs, dc,
                mosi,
                sclk,
                rst) {};

    sDOS_Adafruit_ST7735_Shim(int8_t cs, int8_t dc, int8_t rst) : Adafruit_ST7735(cs, dc, rst) {};
#if !defined(ESP8266)

    sDOS_Adafruit_ST7735_Shim(SPIClass *spiClass, int8_t cs, int8_t dc, int8_t rst) : Adafruit_ST7735(spiClass, cs, dc,
                rst) {};
#endif // end !ESP8266

    void setColumnOffset(int offset) {
        _colstart = offset;
    };

    void setRowOffset(int offset) {
        _rowstart = offset;
    };

    void sendShimCommand(uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes) {
        sendCommand(commandByte, dataBytes, numDataBytes);
    };

    void init(uint8_t width, uint8_t height, uint8_t colstart, uint8_t rowstart) {
        _height = height;
        _width = width;

        commonInit(tft160x80_1);
        displayInit(tft160x80_2);
        _colstart = 26;
        _rowstart = 1;
        displayInit(tft160x80_3);

        uint8_t data = 0xC0;
        sendCommand(ST77XX_MADCTL, &data, 1);
        setRotation(0);

    };
};
