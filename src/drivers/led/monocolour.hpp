#include "kern_inc.h"
#include "abstracts/driver.hpp"

class sDOS_LED_MONO : public sDOS_Abstract_Driver {
public:
    sDOS_LED_MONO(Debugger &debugger, EventsManager &eventsManager, gpio_num_t gpio)
            : _debugger(debugger), _eventsManager(eventsManager), _gpio(gpio) {};

    void setup() {
        _debugger.Debug(_component, "GPIO %d, PWM Channel %d", _gpio, _pwmChannel);
        pinMode(_gpio, OUTPUT);
        ledcSetup(_pwmChannel, 5000, 8);
        ledcAttachPin(_gpio, _pwmChannel);
    };

    bool isActive() {
        return sDOS_LED_MONO::_brightness != sDOS_LED_MONO::_brightnessPrev;
    };

    void loop() {
        updateBrightness();
    };

    void updateBrightness() {
        ledcWrite(_pwmChannel, sDOS_LED_MONO::_brightness);
        sDOS_LED_MONO::_brightnessPrev = sDOS_LED_MONO::_brightness;
    };

    void setBrightness(unsigned int brightness) {
        sDOS_LED_MONO::_brightness = brightness;
        updateBrightness();
    }

    String getName() { return _component; };

private:
    String _component = "LEDMONO";
    Debugger _debugger;
    EventsManager _eventsManager;
    gpio_num_t _gpio;
    int _pwmChannel = sDOS_LED_MONO::getUnusedPWMChannel();
    static unsigned int _brightness;   // 0-255 brightness
    static unsigned int _brightnessPrev;   // 0-255 brightness

};

unsigned int sDOS_LED_MONO::_brightness = 255;
unsigned int sDOS_LED_MONO::_brightnessPrev = 0;