#include "kern_inc.h"
#include "abstracts/driver.hpp"

class SDOS_LED_MONO : public sDOS_Abstract_Driver
{
    public:
        SDOS_LED_MONO(Debugger &debugger, EventsManager &eventsManager, gpio_num_t gpio) 
            : _debugger(debugger), _eventsManager(eventsManager), _gpio(gpio){};

        void setup() {
            _debugger.Debug(_component, "GPIO %d, PWM Channel %d", _gpio, _pwmChannel);
            pinMode(_gpio, OUTPUT);
            ledcSetup(_pwmChannel, 5000, 8);
            ledcAttachPin(_gpio, _pwmChannel);
        };

        boolean isActive(){
            return SDOS_LED_MONO::_brightness != SDOS_LED_MONO::_brightnessPrev;
        };

        void loop() {
            updateBrightness();
        };

        void updateBrightness(){
            _debugger.Debug(_component, "Mono LED (gpio %d) brightness: %d/255", SDOS_LED_MONO::_gpio, SDOS_LED_MONO::_brightness);
            ledcWrite(_pwmChannel, SDOS_LED_MONO::_brightness);
            SDOS_LED_MONO::_brightnessPrev = SDOS_LED_MONO::_brightness;
        };

        void setBrightness(unsigned int brightness){
            SDOS_LED_MONO::_brightness = brightness;
            updateBrightness();
        }

        String getName(){ return _component; };

    private:
        String _component = "LEDMONO";
        Debugger _debugger;
        EventsManager _eventsManager;
        gpio_num_t _gpio;
        int _pwmChannel = SDOS_LED_MONO::getUnusedPWMChannel();
        static unsigned int _brightness;   // 0-255 brightness
        static unsigned int _brightnessPrev;   // 0-255 brightness
        
};

unsigned int SDOS_LED_MONO::_brightness = 255;
unsigned int SDOS_LED_MONO::_brightnessPrev = 0;
