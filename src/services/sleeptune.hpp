#pragma once
#include "kern_inc.h"
#include "abstracts/service.hpp"

#ifndef SLEEPTUNE_LOOPS_PER_SECOND
#define SLEEPTUNE_LOOPS_PER_SECOND 5
#endif
#ifndef SLEEPTUNE_LOOPS_PER_SECOND_VARIATION
#define SLEEPTUNE_LOOPS_PER_SECOND_VARIATION 1
#endif

class sDOS_SLEEPTUNE : public sDOS_Abstract_Service {
private:
    String _component = "SLPTUNE";
    Debugger *_debugger;
    EventsManager *_events;
    WiFiManager *_wifi;
    BluetoothManager *_bluetooth;

    int _loopPerSecondCount = 0;
    int _tuningStep = SLEEPTUNE_TUNING_STEP_MS;
    int _sleepMs = SLEEPTUNE_INITIAL_SLEEP_MS;
    unsigned int _actualSecondLengthMs = 0;
    static unsigned long _micros;
    std::list<int> _awakeTimes;
    static unsigned int _longTermAverageMs ;

public:
    sDOS_SLEEPTUNE(Debugger *debugger, EventsManager *events, WiFiManager *wifi,
                   BluetoothManager *bluetooth)
        : sDOS_Abstract_Service(debugger, events), _wifi(wifi), _bluetooth(bluetooth) {
        //debugger->Debug(_component, "Construct");
    };

    void setup() {};

    void loop() {
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

            //_debugger->Debug(_component, "Going to sleep for %dmS!", _sleepMs);

            if (esp_err_t err = esp_sleep_enable_timer_wakeup(_sleepMs * 1000) != ESP_OK) {
                _debugger->Debug(_component, "Failed to call esp_sleep_enable_timer_wakeup, reason: %s",
                                esp_err_to_name(err));
            }

            if (esp_err_t err = esp_sleep_enable_uart_wakeup(0) != ESP_OK) {
                _debugger->Debug(_component, "Failed to call esp_sleep_enable_uart_wakeup, reason: %s",
                                esp_err_to_name(err));
            }

            if (esp_err_t err = esp_sleep_enable_gpio_wakeup() != ESP_OK) {
                _debugger->Debug(_component, "Failed to call esp_sleep_enable_gpio_wakeup, reason: %s",
                                esp_err_to_name(err));
            }

            unsigned long timeAsleep = micros();
            esp_err_t lightSleepReturn = esp_light_sleep_start();
            if (lightSleepReturn == ESP_OK) {
                timeAsleep = (micros() - timeAsleep) / 1000;
                //_debugger->Debug(_component, "Woke from light sleep of %dMs\n", _sleepMs);
                esp_sleep_wakeup_cause_t wakeup_reason;
                wakeup_reason = esp_sleep_get_wakeup_cause();
                switch (wakeup_reason) {
                case ESP_SLEEP_WAKEUP_EXT0      :
                    _debugger->Debug(_component, "Wakeup after %dMs (scheduled for %dMs) caused by EXT1", timeAsleep,
                                    _sleepMs);
                    break;
                case ESP_SLEEP_WAKEUP_EXT1      :
                    _debugger->Debug(_component, "Wakeup after %dMs (scheduled for %dMs) caused by EXT1", timeAsleep,
                                    _sleepMs);
                    break;
                case ESP_SLEEP_WAKEUP_UART      :
                    _debugger->Debug(_component, "Wakeup after %dMs (scheduled for %dMs) caused by UART", timeAsleep,
                                    _sleepMs);
                    break;
                case ESP_SLEEP_WAKEUP_TIMER     :
                    break; //_debugger->Debug(_component, "Wakeup after %dMs (scheduled for %dMs) caused by timer",                 timeAsleep, sleepUS); break;
                case ESP_SLEEP_WAKEUP_TOUCHPAD  :
                    _debugger->Debug(_component, "Wakeup after %dMs (scheduled for %dMs) caused by touchpad", timeAsleep,
                                    _sleepMs);
                    break;
                case ESP_SLEEP_WAKEUP_ULP       :
                    _debugger->Debug(_component, "Wakeup after %dMs (scheduled for %dMs) caused by ULP program",
                                    timeAsleep, _sleepMs);
                    break;
                case ESP_SLEEP_WAKEUP_GPIO      :
                    break; //_debugger->Debug(_component, "Wakeup after %dMs (scheduled for %dMs) caused by GPIO",                  timeAsleep, sleepUS); break;
                default                         :
                    _debugger->Debug(_component, "Wakeup after %dMs (scheduled for %dMs) was not caused by light sleep",
                                    timeAsleep, _sleepMs);
                    break;
                }
            } else {
                _debugger->Debug(_component, "Failed to light sleep of %dMs: %s\n", _sleepMs,
                                esp_err_to_name(lightSleepReturn));
            }
        }
    };

    bool isActive() override {
        return isSleepPossible();
    };

    bool isSleepPossible() {
        return WiFiManager::canSleep()
               && !(bluetoothState != BT_DISABLED)
               && BluetoothManager::getRequestCount() == 0
               ;
    };

    String getName() {
        return _component;
    };

    static unsigned int getAwakePercentagePerTick() {
        return _longTermAverageMs / 10;
    };

private:

    void oncePerSecond() {
        _actualSecondLengthMs = millis() - _actualSecondLengthMs;
        // Ignore when the processor shits itself and gets 1 loop out in a second.
        if (_loopPerSecondCount > 1) {

            // Calculate how much time per-second we spent asleep
            unsigned int _timeSpentAsleepMs = _sleepMs * _loopPerSecondCount;
            int _timeSpentAwakeMs = _actualSecondLengthMs - _timeSpentAsleepMs;

            _awakeTimes.push_back(_timeSpentAwakeMs);
            if(_awakeTimes.size() > 100) {
                _awakeTimes.pop_front();
            }

            sDOS_SLEEPTUNE::_longTermAverageMs = std::accumulate(std::begin(_awakeTimes), std::end(_awakeTimes),0) / _awakeTimes.size();

            /*_debugger->Debug(
                    _component,
                    "Second took: %dms. SleepTime: %dms. Asleep: %dms. Awake: %dms. Average Awake: %dms",
                    _actualSecondLengthMs,
                    _sleepMs,
                    _timeSpentAsleepMs,
                    _timeSpentAwakeMs,
                    _longTermAverageMs
            );*/

            // Adjust sleep timing
            if (_loopPerSecondCount > SLEEPTUNE_LOOPS_PER_SECOND + SLEEPTUNE_LOOPS_PER_SECOND_VARIATION &&
                    _sleepMs <= 750 - _tuningStep) {
                _sleepMs = _sleepMs + _tuningStep;
                //_debugger->Debug(_component, "Loop per second: %d/s (too fast). Increasing tuned sleep to %dms.",
                //                _loopPerSecondCount, _sleepMs);
                _eventsManager->trigger("sleeptune_adjust", _sleepMs);
            } else if (_loopPerSecondCount < SLEEPTUNE_LOOPS_PER_SECOND - SLEEPTUNE_LOOPS_PER_SECOND_VARIATION && _sleepMs > 0) {
                _sleepMs = _sleepMs - _tuningStep;
                _sleepMs = max(_sleepMs, 0);
                //_debugger->Debug(_component, "Loop per second: %d/s (too slow). Decreasing tuned sleep to %dms.",
                //                _loopPerSecondCount, _sleepMs);
                _eventsManager->trigger("sleeptune_adjust", _sleepMs);
            }
        }
        _actualSecondLengthMs = millis();
        _loopPerSecondCount = 0;
    }
};

unsigned long sDOS_SLEEPTUNE::_micros = 0;
unsigned int sDOS_SLEEPTUNE::_longTermAverageMs = 0;;