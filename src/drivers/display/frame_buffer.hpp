#include "kern_inc.h"
#include "abstracts/driver.hpp"
#include <vector> // for 2D vector 
using namespace std; 

class sDOS_FrameBuffer : public sDOS_Abstract_Driver{
    public:
        enum class ColourDepth { DEPTH_1_BIT, DEPTH_16_BIT };
        sDOS_FrameBuffer(Debugger &debugger, EventsManager &eventsManager, AbstractDisplay &display) 
            : _debugger(debugger), _eventsManager(eventsManager), _display(display){};

        void init(
            uint16_t width, 
            uint16_t height, 
            sDOS_FrameBuffer::ColourDepth colourDepth = ColourDepth::DEPTH_16_BIT
        ){
            _width = width;
            _height = height;
            _colourDepth = colourDepth;
            uint16_t pixelCount = _width * _height;

            _debugger.Debug(_component, "Allocating buffer for %d pixels (%d x %d)", pixelCount, _width, _height);
            uint16_t buffSize = 0;
            uint16_t pixelNum = 0;

            if(_colourDepth == ColourDepth::DEPTH_16_BIT){
                vector<vector<uint16_t>> _buffer16bit(width, vector<uint16_t> (height, 0));
                for(uint16_t x = 0; x < _width; x++){
                    for(uint16_t y = 0; y < _height; y++){
                        uint8_t value = pixelNum % 255;
                        _buffer16bit[x][y] = value;
                        pixelNum++;
                    }
                    buffSize+= _buffer16bit[x].size();
                }
                _buffer8bit_original = _buffer16bit;
            }else if(_colourDepth == ColourDepth::DEPTH_1_BIT){
                vector<vector<bool>> _buffer1bit(width, vector<bool> (height,0));
                for(uint16_t x = 0; x < _width; x++){
                    for(uint16_t y = 0; y < _height; y++){
                        _buffer1bit[x][y] = pixelNum % 2;
                        pixelNum++;
                    }
                    buffSize+= _buffer1bit[x].size();
                }
                _buffer1bit_original = _buffer1bit;
            }
            _debugger.Debug(_component, "Allocated %d bytes to double-buffer", buffSize * 2);
        }

        void setup(){

        };

        void loop(){
            for(uint16_t x = 0; x < _width; x++){
                for(uint16_t y = 0; y < _height; y++){
                    if(_colourDepth == ColourDepth::DEPTH_16_BIT){
                        if(_buffer8bit[x][y] != _buffer8bit_original[x][y]){
                            repaintPixel(x,y, _buffer8bit[x][y]);
                        }
                    }
                }
            }
        };

        virtual String getName(){ return _component; };

        /**
         * Set the pixel in the buffer - this does not trigger a repaint.
         */
        void setPixel(uint16_t x, uint16_t y, uint16_t newValue){
            _buffer8bit[x][y] = newValue;
        }


    private:
        Debugger _debugger;
        EventsManager _eventsManager;
        AbstractDisplay _display;
        String _component = "framebuffer";
        vector<vector<uint16_t>> _buffer8bit;
        vector<vector<uint16_t>> _buffer8bit_original;
        vector<vector<bool>> _buffer1bit;
        vector<vector<bool>> _buffer1bit_original;
        ColourDepth _colourDepth;
        uint16_t _width;
        uint16_t _height;
        void repaintPixel(uint16_t x, uint16_t y, uint16_t updatedValue){
            _debugger.Debug(_component, "Update Pixel %dx%d", x, y);
            _display.writePixel(x,y,updatedValue);
            _buffer8bit_original[x][y] = updatedValue;
        };

        
        
};