#include "kern_inc.h"
#include "abstracts/service.hpp"

#ifndef SLEEPTUNE_LOOPS_PER_SECOND
#define SLEEPTUNE_LOOPS_PER_SECOND 5
#endif
#ifndef SLEEPTUNE_LOOPS_PER_SECOND_VARIATION
#define SLEEPTUNE_LOOPS_PER_SECOND_VARIATION 1
#endif

class sDOS_SLEEPTUNE : public sDOS_Abstract_Service {

public:
    sDOS_SLEEPTUNE(Debugger &debugger, EventsManager &eventsManager, WiFiManager *wifi, BluetoothManager *bluetooth);

    void setup();

    void loop();

    bool isActive() override;

    bool isSleepPossible();

    String getName() { return _component; };

private:
    String _component = "SLPTUNE";
    Debugger _debugger;
    EventsManager _events;
    WiFiManager *_wifi;
    BluetoothManager *_bluetooth;

    void oncePerSecond();

    int _loopPerSecondCount = 0;
    int _tuningStep = SLEEPTUNE_TUNING_STEP_MS;
    int _sleepMs = SLEEPTUNE_INITIAL_SLEEP_MS;
    static unsigned long _micros;
};

unsigned long sDOS_SLEEPTUNE::_micros = 0;

sDOS_SLEEPTUNE::sDOS_SLEEPTUNE(Debugger &debugger, EventsManager &events, WiFiManager *wifi,
                               BluetoothManager *bluetooth)
        : _debugger(debugger), _events(events), _wifi(wifi), _bluetooth(bluetooth) {};

void sDOS_SLEEPTUNE::setup() {
    _loopPerSecondCount = 0;
};

bool sDOS_SLEEPTUNE::isActive() {
    bool possible = isSleepPossible();
    //_debugger.Debug(_component, "is Sleep possible? %s", possible ? "yes" : "no");
    return possible;
}

bool sDOS_SLEEPTUNE::isSleepPossible() {
    return _wifi->canSleep() && !(bluetoothState != BT_DISABLED) && BluetoothManager::getRequestCount() == 0;
}

void sDOS_SLEEPTUNE::loop() {
    /*Serial.printf(
        "_micros: %d\nmicros(): %d\ncheck? %s\n", 
        sDOS_SLEEPTUNE::_micros, 
        micros(), 
        sDOS_SLEEPTUNE::_micros + 1000000 <= micros() ? "yes" : "no"
    );*/

    if (sDOS_SLEEPTUNE::_micros == 0 || sDOS_SLEEPTUNE::_micros > micros() ||
        sDOS_SLEEPTUNE::_micros + 1000000 <= micros()) {
        sDOS_SLEEPTUNE::_micros = micros();
        oncePerSecond();
    }
    _loopPerSecondCount++;

    if (_sleepMs > 0) {
        int sleepUS = _sleepMs * 1000;
        //_debugger.Debug(_component, "Going to sleep for %dmS!", _sleepMs);

        if (esp_err_t err = esp_sleep_enable_timer_wakeup(sleepUS) != ESP_OK) {
            _debugger.Debug(_component, "Failed to call esp_sleep_enable_timer_wakeup, reason: %s",
                            esp_err_to_name(err));
        }

        if (esp_err_t err = esp_sleep_enable_uart_wakeup(0) != ESP_OK) {
            _debugger.Debug(_component, "Failed to call esp_sleep_enable_uart_wakeup, reason: %s",
                            esp_err_to_name(err));
        }

        if (esp_err_t err = esp_sleep_enable_gpio_wakeup() != ESP_OK) {
            _debugger.Debug(_component, "Failed to call esp_sleep_enable_gpio_wakeup, reason: %s",
                            esp_err_to_name(err));
        }

        unsigned long timeAsleep = micros();
        esp_err_t lightSleepReturn = esp_light_sleep_start();
        if (lightSleepReturn == ESP_OK) {
            timeAsleep = micros() - timeAsleep;
            //_debugger.Debug(_component, "Woke from light sleep of %d\n", sleepUS);
            esp_sleep_wakeup_cause_t wakeup_reason;
            wakeup_reason = esp_sleep_get_wakeup_cause();
            switch (wakeup_reason) {
                case ESP_SLEEP_WAKEUP_EXT0      :
                    _debugger.Debug(_component, "Wakeup after %duS (scheduled for %duS) caused by EXT1", timeAsleep,
                                    sleepUS);
                    break;
                case ESP_SLEEP_WAKEUP_EXT1      :
                    _debugger.Debug(_component, "Wakeup after %duS (scheduled for %duS) caused by EXT1", timeAsleep,
                                    sleepUS);
                    break;
                case ESP_SLEEP_WAKEUP_UART      :
                    _debugger.Debug(_component, "Wakeup after %duS (scheduled for %duS) caused by UART", timeAsleep,
                                    sleepUS);
                    break;
                case ESP_SLEEP_WAKEUP_TIMER     :
                    break; //_debugger.Debug(_component, "Wakeup after %duS (scheduled for %duS) caused by timer",                 timeAsleep, sleepUS); break;
                case ESP_SLEEP_WAKEUP_TOUCHPAD  :
                    _debugger.Debug(_component, "Wakeup after %duS (scheduled for %duS) caused by touchpad", timeAsleep,
                                    sleepUS);
                    break;
                case ESP_SLEEP_WAKEUP_ULP       :
                    _debugger.Debug(_component, "Wakeup after %duS (scheduled for %duS) caused by ULP program",
                                    timeAsleep, sleepUS);
                    break;
                case ESP_SLEEP_WAKEUP_GPIO      :
                    break; //_debugger.Debug(_component, "Wakeup after %duS (scheduled for %duS) caused by GPIO",                  timeAsleep, sleepUS); break;
                default                         :
                    _debugger.Debug(_component, "Wakeup after %duS (scheduled for %duS) was not caused by light sleep",
                                    timeAsleep, sleepUS);
                    break;
            }
        } else {
            _debugger.Debug(_component, "Failed to light sleep of %duS: %s\n", sleepUS,
                            esp_err_to_name(lightSleepReturn));
        }
    }
}

void sDOS_SLEEPTUNE::oncePerSecond() {
    // Ignore when the processor shits itself and gets 1 loop out in a second.
    if (_loopPerSecondCount > 1) {
        if (_loopPerSecondCount > SLEEPTUNE_LOOPS_PER_SECOND + SLEEPTUNE_LOOPS_PER_SECOND_VARIATION &&
            _sleepMs <= 750 - _tuningStep) {
            _sleepMs = _sleepMs + _tuningStep;
            //_debugger.Debug(_component, "Loop per second: %d/s (too fast). Increasing tuned sleep to %dms.",
            //                _loopPerSecondCount, _sleepMs);
            _events.trigger("sleeptune_adjust", _sleepMs);
        } else if (_loopPerSecondCount < SLEEPTUNE_LOOPS_PER_SECOND - SLEEPTUNE_LOOPS_PER_SECOND_VARIATION && _sleepMs > 0) {
            _sleepMs = _sleepMs - _tuningStep;
            _sleepMs = max(_sleepMs, 0);
            //_debugger.Debug(_component, "Loop per second: %d/s (too slow). Decreasing tuned sleep to %dms.",
            //                _loopPerSecondCount, _sleepMs);
            _events.trigger("sleeptune_adjust", _sleepMs);
        }
    }
    _loopPerSecondCount = 0;
}