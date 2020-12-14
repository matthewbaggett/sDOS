#pragma once
#include "kern_inc.h"
#include "abstracts/display.hpp"
#include <Adafruit_GFX.h>  // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <shims/Adafruit_ST7789_shim.hpp>

#include <SPI.h>

#ifndef DISPLAY_OFFSET_ROWS
#define DISPLAY_OFFSET_ROWS 0
#endif
#ifndef DISPLAY_OFFSET_COLS
#define DISPLAY_OFFSET_COLS 0
#endif

class sDOS_DISPLAY_ST7789 : public AbstractDisplay {
public:
    sDOS_DISPLAY_ST7789(Debugger *debugger, EventsManager *eventsManager, sDOS_SPI *sdos_spi)
        : _debugger(debugger), _eventsManager(eventsManager), _sdos_spi(sdos_spi) {};

    void setup() {
        _debugger->Debug(_component, "setup()");
        setupBacklight();
        setupReset();
        setupScreen();
    };

    void setupBacklight() {
        pinMode(ST77XX_BL, OUTPUT);
        ledcSetup(_pwmChannel, 5000, 8);
        ledcAttachPin(ST77XX_BL, _pwmChannel);
    };

    void setupReset() {
        pinMode(ST77XX_RST, OUTPUT);
        reset();
    };

    void reset() {
        digitalWrite(ST77XX_RST, LOW);
        delay(30);
        digitalWrite(ST77XX_RST, HIGH);
    };

    void setupScreen() {
        _tft.init(DISPLAY_WIDTH, DISPLAY_HEIGHT, SPI_MODE0, DISPLAY_OFFSET_ROWS, DISPLAY_OFFSET_COLS);
#ifdef ST77XX_ROTATION
        _tft.setRotation(ST77XX_ROTATION);
#endif
        _tft.fillScreen(ST77XX_BLACK);
    };

    void writePixel(uint16_t x, uint16_t y, uint16_t updatedValue) {
        //_debugger->Debug(_component, "writePixel(%d,%d,%d)", x, y, updatedValue);
        _tft.writePixel(x, y, updatedValue);
    };

    virtual void writePixels(uint16_t *colors, uint32_t len, bool block = true, bool bigEndian = false) {
        _tft.writePixels(colors, len, block, bigEndian);
    };

    virtual void setCursor(int16_t x, int16_t y) {
        _tft.setCursor(x, y);
    };

    void loop() {
        updateBacklight();
    };

    void beginRedraw() {
        _debugger->Debug(_component, "beginRedraw()");
        _tft.startWrite();
    };

    void commitRedraw() {
        _debugger->Debug(_component, "commitRedraw()");
        _tft.endWrite();
    };

    void updateBacklight() {
        ledcWrite(_pwmChannel, sDOS_DISPLAY_ST7789::_displayOn ? sDOS_DISPLAY_ST7789::_backlightBrightness : 0);
        //_debugger->Debug(_component, "Backlight %d%%", sDOS_DISPLAY_ST7789::_backlightBrightness);
    };

    void setBacklight(unsigned int backlight) {
        sDOS_DISPLAY_ST7789::_backlightBrightness = backlight;
    };
    void setEnabled(bool on) {
        sDOS_DISPLAY_ST7789::_displayOn = on;
    };

    String getName() {
        return _component;
    };


protected:
    String _component = "ST7789";
    Debugger * _debugger;// @todo refactor into base class
    EventsManager * _eventsManager;// @todo refactor into base class
    sDOS_SPI *_sdos_spi;
    static bool _displayOn;
    static unsigned int _backlightBrightness;   // 0-255 brightness
    sDOS_Adafruit_ST7789_Shim _tft = sDOS_Adafruit_ST7789_Shim(ST77XX_CS, ST77XX_DC, SPI_MOSI, SPI_SCLK, ST77XX_RST);
    const uint16_t _Display_Color_Black = 0x0000;
    const uint16_t _Display_Color_Blue = 0x001F;
    const uint16_t _Display_Color_Red = 0xF800;
    const uint16_t _Display_Color_Green = 0x07E0;
    const uint16_t _Display_Color_Cyan = 0x07FF;
    const uint16_t _Display_Color_Magenta = 0xF81F;
    const uint16_t _Display_Color_Yellow = 0xFFE0;
    const uint16_t _Display_Color_White = 0xFFFF;
    int _pwmChannel = sDOS_DISPLAY_ST7789::getUnusedPWMChannel();
};

unsigned int sDOS_DISPLAY_ST7789::_backlightBrightness = 255;
bool sDOS_DISPLAY_ST7789::_displayOn = true;
