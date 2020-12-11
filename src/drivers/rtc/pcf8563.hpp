#include "kern_inc.h"
#include "abstracts/rtc.hpp"
#include <RTClib.h>

class sDOS_PCF8563 : public AbstractRTC {
public:
    sDOS_PCF8563(Debugger &debugger, EventsManager &eventsManager, sDOS_I2C *i2c)
        : _debugger(debugger), _events(eventsManager), _i2c(i2c) {};

    void setup() {
        _events.trigger(F("rtc_enable"));
        pinMode(PIN_INTERRUPT_PCF8563, INPUT);
        attachInterrupt(PIN_INTERRUPT_PCF8563, sDOS_PCF8563::interrupt, FALLING);
        gpio_wakeup_enable(PIN_INTERRUPT_PCF8563, GPIO_INTR_LOW_LEVEL);
        TwoWire wire = _i2c->getWire();
        // rtc lib can't take _wire as an argument, sadly.
        _rtc.begin();

        if (_rtc.isrunning()) {
            _events.trigger(F("rtc_ready"));
            _debugger.Debug(_component, "RTC Startup time: %s", _rtc.now().toStr());
        } else {
            _events.trigger(F("rtc_fail"));
            return;
        }
    };

    void loop() {
        if (sDOS_PCF8563::hasInterruptOccured()) {
            _events.trigger(F("rtc_interrupt"));
        }
    };

    void setAlarmInMinutes(int minutes) {
        DateTime alarm = _rtc.now();
        alarm.setminute(alarm.minute() + minutes);
        _events.trigger(F("rtc_alarm_set"), alarm);
        _rtc.set_alarm(alarm, {AE_M, 0, 0, 0});
        _rtc.on_alarm();
    };

    void setAlarmInSeconds(int seconds) {
        DateTime alarm = _rtc.now();
        alarm.setsecond(alarm.second() + seconds);
        _events.trigger(F("rtc_alarm_set"), alarm);
        _rtc.set_alarm(alarm, {AE_M, 0, 0, 0});
        _rtc.on_alarm();
    };

    void setTime(DateTime &newTime) {
        _rtc.adjust(newTime);
        _events.trigger("rtc_set", String(newTime.toStr()));
    };

    DateTime getTime() {
        return _rtc.now();
    }

    String getName() {
        return _component;
    };

private:
    String _component = "rtc";

    static void interrupt() {
        _interruptTriggered = true;
    };

    static bool hasInterruptOccured() {
        if (_interruptTriggered) {
            _interruptTriggered = false;
            return true;
        }
        return false;
    };

    static bool _interruptTriggered;
    Debugger _debugger;
    EventsManager _events;
    sDOS_I2C *_i2c;
    PCF8563 _rtc;
};

bool sDOS_PCF8563::_interruptTriggered = false;





