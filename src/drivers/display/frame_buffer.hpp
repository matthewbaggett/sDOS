#ifndef SDOS_FRAMEBUFFER_HPP
#define SDOS_FRAMEBUFFER_HPP

#include "kern_inc.h"
#include "abstracts/driver.hpp"
#include "abstracts/display.hpp"
#include "services/cpu_scaler.hpp"
#include <vector> // for 2D vector
#include <colours.h>

using namespace std;


class sDOS_FrameBuffer : public sDOS_Abstract_Driver {
public:

    struct DirtyPixel{
        uint16_t _x;
        uint16_t _y;
    };

    struct Coordinate{
        uint16_t _x;
        uint16_t _y;
        Coordinate(uint16_t x, uint16_t y){
            _x = x;
            _y = y;
        };
        Coordinate move(int16_t x, int16_t y){
            Coordinate newCoord(_x + x, _y + y);
            return newCoord;
        }
    };

    struct Colour{
        uint8_t _red;
        uint8_t _green;
        uint8_t _blue;
        uint16_t _565;
        Colour(uint8_t red, uint8_t green, uint8_t blue){
            _red = red;
            _green = green;
            _blue = blue;
            _565 = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
        }
    };

    enum class ColourDepth {
        DEPTH_1_BIT, DEPTH_16_BIT
    };

    sDOS_FrameBuffer(Debugger &debugger, EventsManager &eventsManager, AbstractDisplay *display,
                     sDOS_CPU_SCALER *cpuScaler)
            : _debugger(debugger), _eventsManager(eventsManager), _display(display), _cpuScaler(cpuScaler) {};

    void setup() {};

    AbstractDisplay * getDisplay(){ return _display; };

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
        _debugger.Debug(
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
            //_debugger.Debug(_component, "Allocated row %d of %d, %dKB ram free", x, _height, ESP.getFreeHeap() / 1024);
        }
        _everyPixelDirty = true;
        _cpuScaler->onDemand(false);

        buffSize = ramBefore - ESP.getFreeHeap();
        _debugger.Debug(_component, "Allocated %s%dKB%s to double-buffer", COL_RED, buffSize / 1024, COL_RESET);
    }

    void loop() {
        if (_everyPixelDirty) {
            repaintEntireFrame();
        }else if(_dirtyPixels.size() > 0) {
            repaintDirtyPixels();
        }

    };

    virtual String getName() { return "fbuff"; };

    /**
     * Set the pixel in the buffer.
     * this does not trigger a repaint.
     */
    void setPixel(uint16_t x, uint16_t y, uint16_t colour565) {
        sDOS_FrameBuffer::DirtyPixel dirtyPixel{};
        dirtyPixel._x = x;
        dirtyPixel._y = y;
        _dirtyPixels.push_back(dirtyPixel);
        _pixBuf[x][y] = colour565;
    };

    void setPixel(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue) {
        setPixel(x, y, packColour565(red, green, blue));
    };

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour565){
        _cpuScaler->onDemand(true);
        _debugger.Debug(_component, "drawLine(%d,%d,%d,%d)", x0, y0, x1, y1);
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
        }
        _cpuScaler->onDemand(false);
    };

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t red, uint16_t green, uint16_t blue){
        drawLine(x0,y0,x1,y1, packColour565(red,green,blue));
    };

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, sDOS_FrameBuffer::Colour colour) {
        drawLine(x0, y0, x1, y1, colour._565);
    }

    void drawLine(sDOS_FrameBuffer::Coordinate a, sDOS_FrameBuffer::Coordinate b, sDOS_FrameBuffer::Colour colour){
        drawLine(a._x,a._y,b._x,b._y, colour._565);
    }

    /**
     * Set all pixels in the buffer to a specific colour.
     * This does not trigger a repaint.
     */
    void setAll(uint16_t colour565) {
        _cpuScaler->onDemand(true);
        for (uint16_t x = 0; x < _height; x++) {
            for (uint16_t y = 0; y < _width; y++) {
                _pixBuf[x][y] = colour565;
            }
            yield();
        }
        _cpuScaler->onDemand(false);
        _everyPixelDirty = true;
    };

    void setAll(uint8_t red, uint8_t green, uint8_t blue) {
        setAll(packColour565(red, green, blue));
    };

    void setAll(sDOS_FrameBuffer::Colour colour){
        setAll(colour._565);
    }

    bool isActive() {
        return true;
        return (_everyPixelDirty || _dirtyPixels.size() > 0)
                && _display->isActive();
    };

    uint16_t packColour565(uint8_t red, uint8_t green, uint8_t blue) {
        return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
    }

private:
    Debugger _debugger;
    EventsManager _eventsManager;
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
        //_debugger.Debug(_component, "Update Pixel %dx%d on %s", x, y, _display->getName());
        _display->writePixel(x, y, _pixBuf[x][y]);
    };

    void repaintPixelRow(uint16_t x) {
        _display->setCursor(x, 0);
        _display->writePixels(_pixBuf[x], sizeof(_pixBuf[x]), true, true);
    };

    void repaintEntireFrame() {
        _cpuScaler->onDemand(true);
        _display->beginRedraw();
        for(uint16_t y = 0; y < _width; y++){
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

    void repaintDirtyPixels(){
        _debugger.Debug(_component, "There are %d pixels that need repainting", _dirtyPixels.size());
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
sDOS_FrameBuffer::Colour FB_BLACK = sDOS_FrameBuffer::Colour(0,0,0);

#endif