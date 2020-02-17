#ifndef SDOS_FRAMEBUFFER_HPP
#define SDOS_FRAMEBUFFER_HPP

#include "kern_inc.h"
#include "abstracts/driver.hpp"
#include "abstracts/display.hpp"
#include "services/cpu_scaler.hpp"
#include <vector> // for 2D vector
#include <colours.h>

using namespace std;

struct sDOS_FrameBuffer_DirtyPixel{
    uint16_t x;
    uint16_t y;
};

class sDOS_FrameBuffer : public sDOS_Abstract_Driver {
public:
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
        _oldBuf = new uint16_t *[_height];

        uint16_t red = packColour565(255, 0, 0);
        uint16_t green = packColour565(0, 255, 0);
        uint16_t blue = packColour565(0, 0, 255);
        uint16_t white = packColour565(255, 255, 255);

        for (uint16_t x = 0; x < _height; ++x) {
            _pixBuf[x] = new uint16_t[_width];
            _oldBuf[x] = new uint16_t[_width];
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
                _oldBuf[x][y] = 0;
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
        if(_dirtyPixels.size() > 0) {
            _debugger.Debug(_component, "There are %d pixels that need updating", _dirtyPixels.size());
            std::list<sDOS_FrameBuffer_DirtyPixel>::iterator it;
            for (it = _dirtyPixels.begin(); it != _dirtyPixels.end(); ++it) {
                repaintPixel(it->x, it->y);
            }
            _dirtyPixels.clear();
        }
        if (_everyPixelDirty) {
            repaintEntireBuf();
        }
    };

    virtual String getName() { return "fbuff"; };

    /**
     * Set the pixel in the buffer.
     * this does not trigger a repaint.
     */
    void setPixel(uint16_t x, uint16_t y, uint16_t colour565) {
        sDOS_FrameBuffer_DirtyPixel dirtyPixel;
        dirtyPixel.x = x;
        dirtyPixel.y = y;
        _dirtyPixels.push_back(dirtyPixel);
        _pixBuf[x][y] = colour565;
    };

    void setPixel(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue) {
        setPixel(x, y, packColour565(red, green, blue));
    };

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

    bool isActive() {
        return true;
        return (_everyPixelDirty || _dirtyPixels.size() > 0)
                && _display->isActive();
    };

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
    uint16_t **_oldBuf;
    bool _everyPixelDirty = false;
    std::list<sDOS_FrameBuffer_DirtyPixel> _dirtyPixels;

    void repaintPixel(uint16_t x, uint16_t y) {
        //_debugger.Debug(_component, "Update Pixel %dx%d on %s", x, y, _display->getName());
        _display->writePixel(x, y, _pixBuf[x][y]);
        _oldBuf[x][y] = _pixBuf[x][y];
    };

    void repaintPixelRow(uint16_t x) {
        _display->setCursor(x, 0);
        _display->writePixels(_pixBuf[x], sizeof(_pixBuf[x]), true, true);
        _oldBuf[x] = _pixBuf[x];
    };

    void repaintEntireBuf() {
        _cpuScaler->onDemand(true);
        _display->beginRedraw();
        for (uint16_t x = 0; x < _height; x++) {
            _display->setCursor(x, 0);
            _display->writePixels(_pixBuf[x], _width, true, true);
        }
        yield();
        _display->commitRedraw();
        _cpuScaler->onDemand(false);
        _everyPixelDirty = false;
    };

    uint16_t packColour565(uint8_t red, uint8_t green, uint8_t blue) {
        return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
    }
};

#endif