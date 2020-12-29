#pragma once

#include "kern_inc.h"
#include "abstracts/driver.hpp"
#include "abstracts/display.hpp"
#include "services/cpu_scaler.hpp"
#include "drivers/display/pgm-ptr.h"
#include <vector> // for 2D vector
#include <colours.h>

using namespace std;

class sDOS_FrameBuffer : public sDOS_Abstract_Driver {
public:
    struct Colour {
        uint8_t _red;
        uint8_t _green;
        uint8_t _blue;
        uint16_t _565;
        Colour(uint8_t red, uint8_t green, uint8_t blue) {
            _red = red;
            _green = green;
            _blue = blue;
            _565 = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
        }
    };

    struct DirtyPixel {
        uint16_t _x;
        uint16_t _y;
    };

    struct Coordinate {
        uint16_t _x;
        uint16_t _y;
        Coordinate(uint16_t x, uint16_t y) {
            _x = x;
            _y = y;
        };
        Coordinate move(int16_t x, int16_t y) {
            Coordinate newCoord(_x + x, _y + y);
            return newCoord;
        }
    };

    struct Region {
        uint16_t _topLeft_x;
        uint16_t _topLeft_y;
        uint16_t _bottomRight_x;
        uint16_t _bottomRight_y;
        Region(uint16_t tl_x, uint16_t tl_y, uint16_t br_x, uint16_t br_y) {
            _topLeft_x = tl_x;
            _topLeft_y = tl_y;
            _bottomRight_x = br_x;
            _bottomRight_y = br_y;
        };
        Region move(int16_t x, int16_t y) {
            Region newRegion (_topLeft_x + x, _topLeft_y + y, _bottomRight_x + x, _bottomRight_y + y);
            return newRegion;
        }
        String __toString() {
            return "region from (" + String(_topLeft_x) + "," + String(_topLeft_y) + ") to (" + String(_bottomRight_x) + "," + String(_bottomRight_y) + ")";
        }
        uint16_t getHeight() {
            return _bottomRight_y - _topLeft_y;
        }
        uint16_t getWidth() {
            return _bottomRight_x - _topLeft_x;
        }
        Coordinate getTopLeft() {
            return {_topLeft_x, _topLeft_y};
        }
        Coordinate getTopRight() {
            return {_topLeft_x, _bottomRight_y};
        }
        Coordinate getBottomLeft() {
            return {_bottomRight_x, _topLeft_y};
        }
        Coordinate getBottomRight() {
            return {_bottomRight_x, _bottomRight_y};
        }
        void highlight(sDOS_FrameBuffer * _frameBuffer, sDOS_FrameBuffer::Colour colour) {
            _frameBuffer->highlightRegion(this, colour);
        }
    };

    enum class ColourDepth {
        DEPTH_1_BIT, DEPTH_16_BIT
    };

    sDOS_FrameBuffer(Debugger * debugger, EventsManager * eventsManager, AbstractDisplay *display,
                     sDOS_CPU_SCALER *cpuScaler)
        : sDOS_Abstract_Driver(debugger, eventsManager), _display(display), _cpuScaler(cpuScaler) {
    };

    void setup() {
        sDOS_Abstract_Driver::setup();
    };

    void loop() {
        sDOS_Abstract_Driver::loop();
        if (_everyPixelDirty) {
            repaintEntireFrame();
        } else if(_dirtyPixels.size() > 0) {
            repaintDirtyPixels();
        }
    };

    AbstractDisplay * getDisplay() {
        return _display;
    };

    void highlightRegion(Region * region, Colour colour) {
        this->drawLine(region->getTopLeft(), region->getTopRight(), colour);
        this->drawLine(region->getTopRight(), region->getBottomRight(), colour);
        this->drawLine(region->getBottomRight(), region->getBottomLeft(), colour);
        this->drawLine(region->getBottomLeft(), region->getTopLeft(), colour);
    }

    void init(
        uint16_t width,
        uint16_t height,
        sDOS_FrameBuffer::ColourDepth colourDepth = ColourDepth::DEPTH_16_BIT
    ) {
        _width = width;
        _height = height;
        _colourDepth = colourDepth;
        uint32_t ramBefore = ESP.getFreeHeap();
        uint16_t pixelCount = _width * _height;
        uint32_t buffSize = 0;
        _cpuScaler->onDemand(true);
        _debugger->Debug(
            _component,
            "Allocating buffer for %s%d%s pixels (%s%d x %d%s)",
            COL_BLUE, pixelCount, COL_RESET, COL_BLUE, _width, _height, COL_RESET
        );
        uint16_t pixelNum = 0;
        _pixBuf = new uint16_t *[_height];

        uint16_t red = packColour565(255, 0, 0);
        uint16_t green = packColour565(0, 255, 0);
        uint16_t blue = packColour565(0, 0, 255);
        uint16_t white = packColour565(255, 255, 255);

        for (uint16_t x = 0; x < _height; ++x) {
            _pixBuf[x] = new uint16_t[_width];
            for (uint16_t y = 0; y < _width; ++y) {
                if (y <= (_width / 3)) {
                    _pixBuf[x][y] = red;
                } else if (y > (_width / 3) && y <= ((_width / 3) * 2)) {
                    _pixBuf[x][y] = green;
                } else {
                    _pixBuf[x][y] = blue;
                }
                if (y == 1 || y == width - 2 || x == 1 || x == height - 2) {
                    _pixBuf[x][y] = white;
                }
                pixelNum++;
                yield();
            }
            //_debugger->Debug(_component, "Allocated row %d of %d, %dKB ram free", x, _height, ESP.getFreeHeap() / 1024);
        }
        _everyPixelDirty = true;
        _cpuScaler->onDemand(false);

        buffSize = ramBefore - ESP.getFreeHeap();
        _debugger->Debug(_component, "Allocated %s%dKB%s to framebuffer", COL_RED, buffSize / 1024, COL_RESET);
    }



    virtual String getName() {
        return "fbuff";
    };

    /**
     * Set the pixel in the buffer.
     * this does not trigger a repaint.
     */
    void setPixel(uint16_t x, uint16_t y, uint16_t colour565) {
        if(!pixelInBounds(x, y)) {
            //_debugger->Debug(_component, "Tried to write to coordinate out of bounds: %d,%d", x, y);
            return;
        }
        sDOS_FrameBuffer::DirtyPixel dirtyPixel{};
        dirtyPixel._x = x;
        dirtyPixel._y = y;
        _dirtyPixels.push_back(dirtyPixel);
        _pixBuf[x][y] = colour565;
    };

    void setPixel(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue) {
        setPixel(x, y, packColour565(red, green, blue));
    };

    void setPixel(uint16_t x, uint16_t y, Colour colour) {
        setPixel(x,y, colour._565);
    }

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour565) {
        yield();
        _cpuScaler->onDemand(true);
        //_debugger->Debug(_component, "drawLine(%d,%d,%d,%d)", x0, y0, x1, y1);
        int16_t steep = abs(y1 - y0) > abs(x1 - x0);
        if (steep) {
            _swap_int16_t(x0, y0);
            _swap_int16_t(x1, y1);
        }

        if (x0 > x1) {
            _swap_int16_t(x0, x1);
            _swap_int16_t(y0, y1);
        }

        int16_t dx, dy;
        dx = x1 - x0;
        dy = abs(y1 - y0);

        int16_t err = dx / 2;
        int16_t ystep;

        if (y0 < y1) {
            ystep = 1;
        } else {
            ystep = -1;
        }

        for (; x0 <= x1; x0++) {
            if (steep) {
                setPixel(y0, x0, colour565);
            } else {
                setPixel(x0, y0, colour565);
            }
            err -= dy;
            if (err < 0) {
                y0 += ystep;
                err += dx;
            }
            yield();
        }
        if(ESP.getFreeHeap() < 50000) {
            repaintDirtyPixels();
        }
        _cpuScaler->onDemand(false);
    };


    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, sDOS_FrameBuffer::Colour colour) {
        drawLine(x0, y0, x1, y1, colour._565);
    }

    void drawLine(sDOS_FrameBuffer::Coordinate a, sDOS_FrameBuffer::Coordinate b, sDOS_FrameBuffer::Colour colour) {
        drawLine(a._x,a._y,b._x,b._y, colour._565);
    }

    Region drawText(uint16_t x, uint16_t y, const String& text, const GFXfont * gfxFont, sDOS_FrameBuffer::Colour colour) {
        return drawText(x, y, text, gfxFont, colour._565);
    }

    Region drawText(uint16_t x, uint16_t y, const String& text, const GFXfont * gfxFont, uint16_t colour565) {
        _cpuScaler->onDemand(true);
        //_debugger->Debug(_component, "Write %s at %d,%d", text.c_str(), x, y);
        int xAdvance = 0;

        uint16_t tl_x = UINT16_MAX, tl_y = UINT16_MAX, br_x = 0, br_y =0;

        for(int charOffset = 0; charOffset < text.length(); charOffset++) {

            int glyphOffset = text.charAt(charOffset) - gfxFont->first;
            GFXglyph glyph = gfxFont->glyph[glyphOffset];

            // Calculate the affected region
            Region charRegion = drawChar(x + xAdvance, y + gfxFont->yAdvance, text.charAt(charOffset), gfxFont, colour565);
            if(charRegion._topLeft_x < tl_x)     tl_x = charRegion._topLeft_x;
            if(charRegion._topLeft_y < tl_y)     tl_y = charRegion._topLeft_y;
            if(charRegion._bottomRight_x > br_x) br_x = charRegion._bottomRight_x;
            if(charRegion._bottomRight_y > br_y) br_y = charRegion._bottomRight_y;

            xAdvance = xAdvance + glyph.xAdvance;
            yield();
        }
        _cpuScaler->onDemand(false);
        return {tl_x, tl_y, br_x, br_y};
    }

    Region boundText(uint16_t x, uint16_t y, const String& text, const GFXfont * gfxFont) {
        _cpuScaler->onDemand(true);
        _debugger->Debug(_component, "Write %s at %d,%d", text.c_str(), x, y);
        int xAdvance = 0;

        uint16_t tl_x = UINT16_MAX, tl_y = UINT16_MAX, br_x = 0, br_y =0;

        for(int charOffset = 0; charOffset < text.length(); charOffset++) {

            int glyphOffset = text.charAt(charOffset) - gfxFont->first;
            GFXglyph glyph = gfxFont->glyph[glyphOffset];

            // Calculate the affected region
            Region charRegion = boundChar(x + xAdvance, y + gfxFont->yAdvance, text.charAt(charOffset), gfxFont);
            if(charRegion._topLeft_x < tl_x)     tl_x = charRegion._topLeft_x;
            if(charRegion._topLeft_y < tl_y)     tl_y = charRegion._topLeft_y;
            if(charRegion._bottomRight_x > br_x) br_x = charRegion._bottomRight_x;
            if(charRegion._bottomRight_y > br_y) br_y = charRegion._bottomRight_y;

            xAdvance = xAdvance + glyph.xAdvance;
            yield();
        }
        _cpuScaler->onDemand(false);
        return {tl_x, tl_y, br_x, br_y};
    }

    bool pixelInBounds(uint16_t x, uint16_t y) {
        return (x >= 0 && y >= 0 && x < _height && y < _width);
    }

    Region drawChar(uint16_t x, uint16_t y, char c, const GFXfont * gfxFont, uint16_t colour565) {

        c -= (uint8_t)pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c);
        uint8_t *bitmap = pgm_read_bitmap_ptr(gfxFont);

        uint16_t bitmapOffset = pgm_read_word(&glyph->bitmapOffset);
        uint8_t glyphWidth = pgm_read_byte(&glyph->width);
        uint8_t glyphHeight = pgm_read_byte(&glyph->height);
        int8_t xOffset = pgm_read_byte(&glyph->xOffset);
        int8_t yOffset = pgm_read_byte(&glyph->yOffset);

        uint8_t xx = 0, yy = 0, bits = 0, bit = 0;
        // Region min/maxes
        uint16_t topLeftX = UINT16_MAX, topLeftY = UINT16_MAX, bottomRightX = 0, bottomRightY = 0;
        // Number of changed pixels
        uint16_t pixelChangedCount = 0;
        for (yy = 0; yy < glyphHeight; yy++) {
            for (xx = 0; xx < glyphWidth; xx++) {
                if (!(bit++ & 7)) {
                    bits = pgm_read_byte(&bitmap[bitmapOffset++]);
                }
                if (bits & 0x80) {
                    int16_t pixelX = x + xOffset + xx;
                    int16_t pixelY = y + yOffset + yy;

                    if(pixelInBounds(pixelX, pixelY)) {
                        if(pixelX < topLeftX)         topLeftX = pixelX;
                        if(pixelY < topLeftY)         topLeftY = pixelY;
                        if(pixelX > bottomRightX) bottomRightX = pixelX;
                        if(pixelY > bottomRightY) bottomRightY = pixelY;

                        setPixel(pixelX, pixelY, colour565);
                        pixelChangedCount++;
                    } else {
                        //_debugger->Debug(_component, "setPixel(%d,%d) : %s", pixelX, pixelY, "Not in bounds");
                    }
                }
                bits <<= 1;
            }
        }
        return {topLeftX, topLeftY, bottomRightX, bottomRightY};
    }

    Region boundChar(uint16_t x, uint16_t y, char c, const GFXfont * gfxFont) {
        c -= (uint8_t)pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c);
        uint8_t *bitmap = pgm_read_bitmap_ptr(gfxFont);

        uint16_t bitmapOffset = pgm_read_word(&glyph->bitmapOffset);
        uint8_t glyphWidth = pgm_read_byte(&glyph->width);
        uint8_t glyphHeight = pgm_read_byte(&glyph->height);
        int8_t xOffset = pgm_read_byte(&glyph->xOffset);
        int8_t yOffset = pgm_read_byte(&glyph->yOffset);

        uint8_t xx = 0, yy = 0, bits = 0, bit = 0;
        // Region min/maxes
        uint16_t topLeftX = UINT16_MAX, topLeftY = UINT16_MAX, bottomRightX = 0, bottomRightY = 0;
        for (yy = 0; yy < glyphHeight; yy++) {
            for (xx = 0; xx < glyphWidth; xx++) {
                if (!(bit++ & 7)) {
                    bits = pgm_read_byte(&bitmap[bitmapOffset++]);
                }
                if (bits & 0x80) {
                    int16_t pixelX = x + xOffset + xx;
                    int16_t pixelY = y + yOffset + yy;

                    if(pixelInBounds(pixelX, pixelY)) {
                        if(pixelX < topLeftX)         topLeftX = pixelX;
                        if(pixelY < topLeftY)         topLeftY = pixelY;
                        if(pixelX > bottomRightX) bottomRightX = pixelX;
                        if(pixelY > bottomRightY) bottomRightY = pixelY;
                    }
                }
                bits <<= 1;
            }
        }
        return {topLeftX, topLeftY, bottomRightX, bottomRightY};
    }

    /**
     * Set all pixels in the buffer to a specific colour.
     * This does not trigger a repaint.
     */
    void fillEntireFrame(sDOS_FrameBuffer::Colour colour) {
        this->fillRegion(Region(0,0,_height -1, _width -1), colour);
        _everyPixelDirty = true;
    }

    void fillRegion(sDOS_FrameBuffer::Region region, sDOS_FrameBuffer::Colour colour) {
        _cpuScaler->onDemand(true);
        for (uint16_t x = region._topLeft_x; x <= region._bottomRight_x; x++) {
            for (uint16_t y = region._topLeft_y; y <= region._bottomRight_y; y++) {
                if(pixelInBounds(x,y)) {
                    _pixBuf[x][y] = colour._565;
                }
            }
            yield();
        }
        _everyPixelDirty = true;
        _cpuScaler->onDemand(false);
    }

    bool isActive() override {
        return true;
    };

    uint16_t packColour565(uint8_t red, uint8_t green, uint8_t blue) {
        return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
    }

    uint16_t getWidth() const {
        return _width;
    }

    uint16_t getHeight() const {
        return _height;
    }

    void drawXBM(uint16_t x, uint16_t y, uint16_t width, uint16_t height, unsigned char xbmData) {

    }

protected:
    AbstractDisplay *_display;
    sDOS_CPU_SCALER *_cpuScaler;
    String _component = "fbuff";
    ColourDepth _colourDepth;
    uint16_t _width;
    uint16_t _height;
    uint16_t **_pixBuf;
    bool _everyPixelDirty = false;
    std::list<sDOS_FrameBuffer::DirtyPixel> _dirtyPixels;

    void repaintPixel(uint16_t x, uint16_t y) {
        //_debugger->Debug(_component, "Update Pixel %dx%d on %s", x, y, _display->getName());
        _display->writePixel(x, y, _pixBuf[x][y]);
    };

    void repaintPixelRow(uint16_t x) {
        _display->setCursor(x, 0);
        _display->writePixels(_pixBuf[x], sizeof(_pixBuf[x]), true, true);
    };

    void repaintEntireFrame() {
        _cpuScaler->onDemand(true);
        _display->beginRedraw();
        for(uint16_t y = 0; y < _width; y++) {
            for (uint16_t x = 0; x < _height; x++) {
                //_display->setCursor(x, 0);
                //_display->writePixels(_pixBuf[x], _width, true, true);
                _display->writePixel(x,y,_pixBuf[x][y]);
            }
        }
        yield();
        _display->commitRedraw();
        _cpuScaler->onDemand(false);
        _dirtyPixels.clear();
        _everyPixelDirty = false;
    };

    void repaintDirtyPixels() {
        _debugger->Debug(_component, "There are %d pixels that need repainting", _dirtyPixels.size());
        std::list<sDOS_FrameBuffer::DirtyPixel>::iterator it;
        for (it = _dirtyPixels.begin(); it != _dirtyPixels.end(); ++it) {
            repaintPixel(it->_x, it->_y);
        }
        _dirtyPixels.clear();
    }

    void _swap_int16_t(int16_t &a, int16_t &b) {
        int16_t t = a;
        a = b;
        b = t;
    };

    void _swap_uint16_t(uint16_t &a, uint16_t &b) {
        int16_t t = a;
        a = b;
        b = t;
    };
};

sDOS_FrameBuffer::Colour FB_RED = sDOS_FrameBuffer::Colour(0xFF,0,0);
sDOS_FrameBuffer::Colour FB_GREEN = sDOS_FrameBuffer::Colour(0,255,0);
sDOS_FrameBuffer::Colour FB_BLUE = sDOS_FrameBuffer::Colour(0,0,255);
sDOS_FrameBuffer::Colour FB_YELLOW = sDOS_FrameBuffer::Colour(255,255,0);
sDOS_FrameBuffer::Colour FB_CYAN = sDOS_FrameBuffer::Colour(0,255,255);
sDOS_FrameBuffer::Colour FB_PINK = sDOS_FrameBuffer::Colour(255,0,255);
sDOS_FrameBuffer::Colour FB_WHITE = sDOS_FrameBuffer::Colour(255,255,255);
sDOS_FrameBuffer::Colour FB_DARK_GREY = sDOS_FrameBuffer::Colour(50,50,50);
sDOS_FrameBuffer::Colour FB_BLACK = sDOS_FrameBuffer::Colour(0,0,0);
