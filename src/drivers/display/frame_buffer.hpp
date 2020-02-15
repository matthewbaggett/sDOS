#ifndef SDOS_FRAMEBUFFER_HPP
#define SDOS_FRAMEBUFFER_HPP
#include "kern_inc.h"
#include "abstracts/driver.hpp"
#include "abstracts/display.hpp"
#include "services/cpu_scaler.hpp"
#include <vector> // for 2D vector 
using namespace std; 

class sDOS_FrameBuffer : public sDOS_Abstract_Driver{
    public:
        enum class ColourDepth { DEPTH_1_BIT, DEPTH_16_BIT };

        sDOS_FrameBuffer(Debugger &debugger, EventsManager &eventsManager, AbstractDisplay * display, sDOS_CPU_SCALER * cpuScaler) 
            : _debugger(debugger), _eventsManager(eventsManager), _display(display), _cpuScaler(cpuScaler){};

        void setup(){};

        void init(
            uint16_t width, 
            uint16_t height, 
            sDOS_FrameBuffer::ColourDepth colourDepth = ColourDepth::DEPTH_16_BIT
        ){
            _width = width;
            _height = height;
            _colourDepth = colourDepth;
            uint32_t ramBefore = ESP.getFreeHeap();
            uint16_t pixelCount = _width * _height;
            uint32_t buffSize = 0;
            _cpuScaler->onDemand(true);
            _debugger.Debug(_component, "Allocating buffer for %d pixels (%d x %d)", pixelCount, _width, _height);
            uint16_t pixelNum = 0;
            _pixBuf = new uint16_t*[_height];
            _oldBuf = new uint16_t*[_height];

            uint16_t   red = packColour565(255,0,0);
            uint16_t green = packColour565(0,255,0);
            uint16_t  blue = packColour565(0,0,255);
            uint16_t white = packColour565(255,255,255);

            for(uint16_t x = 0; x < _height; ++x) {
                _pixBuf[x] = new uint16_t[_width];
                _oldBuf[x] = new uint16_t[_width];
                for(uint16_t y = 0; y < _width; ++y) {
                    if (y <= (_width / 3)) {
                        _pixBuf[x][y] = red;
                    } else if (y > (_width / 3) && y <= ((_width/3)*2)) {
                        _pixBuf[x][y] = green;
                    } else {
                        _pixBuf[x][y] = blue;
                    }
                    if( y == 1 || y == width - 2 || x == 1 || x == height - 2){
                        _pixBuf[x][y] = white;
                    }
                    _oldBuf[x][y] = 0;
                    pixelNum++;
                    yield();
                }
                //_debugger.Debug(_component, "Allocated row %d of %d, %dKB ram free", x, _height, ESP.getFreeHeap() / 1024);
            }
            _isDirty = true;
            _cpuScaler->onDemand(false);

            buffSize = ramBefore - ESP.getFreeHeap();
            _debugger.Debug(_component, "Allocated %dKB to double-buffer", buffSize / 1024);
        }

        void loop() {
            scanForChanges();
        };

        virtual String getName(){ return "fbuff"; };

        /**
         * Set the pixel in the buffer.
         * this does not trigger a repaint.
         */
        void setPixel(uint16_t x, uint16_t y, uint16_t colour565){
            _pixBuf[x][y] = colour565;
            _isDirty = true;
        };
        void setPixel(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue){
            setPixel(x, y, packColour565(red, green, blue));
        };

        /**
         * Set all pixels in the buffer to a specific colour.
         * This does not trigger a repaint.
         */
        void setAll(uint16_t colour565){
            _cpuScaler->onDemand(true);
            for(uint16_t x = 0; x < _height; x++) {
                for(uint16_t y = 0; y < _width; y++) {
                    _pixBuf[x][y] = colour565;
                }
                yield();
            }
            _cpuScaler->onDemand(false);
            _isDirty = true;
        };
        void setAll(uint8_t red, uint8_t green, uint8_t blue){
            setAll(packColour565(red, green, blue));
        };

        bool isActive() { return _isDirty && _display->isActive(); };

    private:
        Debugger _debugger;
        EventsManager _eventsManager;
        AbstractDisplay * _display;
        sDOS_CPU_SCALER * _cpuScaler;
        String _component = "fbuff";
        ColourDepth _colourDepth;
        uint16_t _width;
        uint16_t _height;
        uint16_t** _pixBuf;
        uint16_t** _oldBuf;
        bool _isDirty = false;
        void repaintPixel(uint16_t x, uint16_t y){
            //_debugger.Debug(_component, "Update Pixel %dx%d on %s", x, y, _display->getName());
            _display->writePixel(x,y,_pixBuf[x][y]);
            _oldBuf[x][y] = _pixBuf[x][y];
        };
        void repaintPixelRow(uint16_t x){
            _display->setCursor(x, 0);
            _display->writePixels(_pixBuf[x], sizeof(_pixBuf[x]), true, true);
            _oldBuf[x] = _pixBuf[x];
        };
        void repaintEntireBuf(){
            _cpuScaler->onDemand(true);
            _display->beginRedraw();

            for(uint16_t x = 0; x < _width; x++) {
                _display->setCursor(x, 0);
                _display->writePixels(_pixBuf[x], _height, true, true);
                yield();
            }
            _display->commitRedraw();
            _cpuScaler->onDemand(false);
        };

        uint16_t packColour565(uint8_t red, uint8_t green, uint8_t blue){
            return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
        }
        bool scanForChanges(){
            repaintEntireBuf();
            return true;
            
            /*
            bool startedWriteSession = false;
            for(uint16_t x = 0; x < _height; x++) {
                for(uint16_t y = 0; y < _width; y++) {
                    //_debugger.Debug(_component, "Pixel: %dx%d = %u", w, h, _pixBuf[w][h]);
                    if(_pixBuf[x][y] != _oldBuf[x][y]) {
                        if(!startedWriteSession) {
                            _cpuScaler->onDemand(true);
                            _display->beginRedraw();
                            startedWriteSession = true;
                        }
                        repaintPixelRow(x);
                        //repaintPixel(x, y);
                    }
                    yield();
                }
                yield();
            }
            _isDirty = false;
            if(startedWriteSession){
                _display->commitRedraw();
                _cpuScaler->onDemand(false);
                return true;
            }
            return false;
            */
        };
};
#endif