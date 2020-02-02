#include "includes.h"

#ifndef SLEEPTUNE_LOOPS_PER_SECOND
#define SLEEPTUNE_LOOPS_PER_SECOND 5
#endif
#ifndef SLEEPTUNE_LOOPS_PER_SECOND_VARIATION
#define SLEEPTUNE_LOOPS_PER_SECOND_VARIATION 1
#endif

class SDOS_SLEEPTUNE
{

public:
    SDOS_SLEEPTUNE(Debugger &debugger, EventsManager &eventsManager, WiFiManager &wifi);
    void setup();
    void loop();
    void oncePerSecond();

private:
    String _component = "SleepTune";
    Debugger _debugger;
    EventsManager _events;
    WiFiManager _wifi;

    int _loopPerSecondCount = 0;
    int _tuningStep = 5;
    int _sleepMs = 0;
    static unsigned long _micros;
};

unsigned long SDOS_SLEEPTUNE::_micros = 0;

SDOS_SLEEPTUNE::SDOS_SLEEPTUNE(Debugger &debugger, EventsManager &events, WiFiManager &wifi)
    : _debugger(debugger), _events(events), _wifi(wifi)
    {};

void SDOS_SLEEPTUNE::setup()
{
    _debugger.Debug(_component, "setup");
    _loopPerSecondCount = 0;
};

void SDOS_SLEEPTUNE::loop()
{
    if(!_wifi.canSleep()){
        return;
    }
    /*Serial.printf(
        "_micros: %d\nmicros(): %d\ncheck? %s\n", 
        SDOS_SLEEPTUNE::_micros, 
        micros(), 
        SDOS_SLEEPTUNE::_micros + 1000000 <= micros() ? "yes" : "no"
    );*/

    if(SDOS_SLEEPTUNE::_micros == 0 || SDOS_SLEEPTUNE::_micros > micros() || SDOS_SLEEPTUNE::_micros + 1000000 <= micros()){
        SDOS_SLEEPTUNE::_micros = micros();
        oncePerSecond();
    }
    _loopPerSecondCount++;

    if (_sleepMs > 0)
    {
        //_debugger.Debug(_component, "Going to sleep!");
        int sleepUS = _sleepMs * 1000;
        #ifdef SLEEPTUNE_WAKEUP_EXT1_BITMASK
        esp_sleep_enable_ext1_wakeup(SLEEPTUNE_WAKEUP_EXT1_BITMASK, ESP_EXT1_WAKEUP_ALL_LOW);
        #endif
        //esp_sleep_enable_touchpad_wakeup();
        esp_sleep_enable_timer_wakeup(sleepUS);
        esp_err_t ret = esp_light_sleep_start();
        if (ret == ESP_OK)
        {
            //_debugger.Debug(_component, "Woke from light sleep of %d\n", sleepUS);
            esp_sleep_wakeup_cause_t wakeup_reason;
            wakeup_reason = esp_sleep_get_wakeup_cause();
            switch(wakeup_reason)
            {
                case 1  : _debugger.Debug(_component, "Wakeup caused by external signal using RTC_IO"); break;
                case 2  : _debugger.Debug(_component, "Wakeup caused by external signal using RTC_CNTL"); break;
                case 3  : break; //_debugger.Debug(_component, "Wakeup caused by timer"); break;
                case 4  : _debugger.Debug(_component, "Wakeup caused by touchpad"); break;
                case 5  : _debugger.Debug(_component, "Wakeup caused by ULP program"); break;
                default : _debugger.Debug(_component, "Wakeup was not caused by light sleep"); break;
            }
            return;
        }
        _debugger.Debug(_component, "Failed to light sleep of %d\n", sleepUS);
    }
}

void SDOS_SLEEPTUNE::oncePerSecond(){
    // Ignore when the processor shits itself and gets 1 loop out in a second.
    if (_loopPerSecondCount > 1)
    {
        if (_loopPerSecondCount > SLEEPTUNE_LOOPS_PER_SECOND + SLEEPTUNE_LOOPS_PER_SECOND_VARIATION)
        {
            _sleepMs = _sleepMs + _tuningStep;
            //_debugger.Debug(_component, "Loop per second: %d/s (too fast). Increasing tuned sleep to %dms.", _loopPerSecondCount, _sleepMs);
            //_events.trigger("sleeptune_adjust", _sleepMs);
        }
        else if (_loopPerSecondCount < SLEEPTUNE_LOOPS_PER_SECOND - SLEEPTUNE_LOOPS_PER_SECOND_VARIATION)
        {
            _sleepMs = _sleepMs - _tuningStep;
            _sleepMs = max(_sleepMs, 0);
            //_debugger.Debug(_component, "Loop per second: %d/s (too slow). Decreasing tuned sleep to %dms.", _loopPerSecondCount, _sleepMs);
            //_events.trigger("sleeptune_adjust", _sleepMs);
        }
    }
    _loopPerSecondCount = 0;
}