#pragma once
#include "kern_inc.h"
#include "rtc.hpp"
#include <RTClib.h>

class sDOS_FAKE_RTC : public sDOS_RTC {
protected:
    String _component = "fakertc";
    RTC_Millis _rtc;
public:
    sDOS_FAKE_RTC(Debugger * debugger, EventsManager * eventsManager)
        : sDOS_RTC(debugger, eventsManager) {
    };

    void setup() {
        DateTime buildTime = DateTime(__DATE__, __TIME__);
        _eventsManager->trigger(F("rtc_enable"));
        _rtc.begin(buildTime);
        _eventsManager->trigger(F("rtc_ready"));
        _debugger->Debug(_component, "RTC Startup time: %s", _rtc.now().toStr());
    };

    void loop() {};

    void setAlarmInMinutes(int minutes) {};

    void setAlarmInSeconds(int seconds) {};

    void setTime(DateTime &newTime) {
        _rtc.adjust(newTime);
        _eventsManager->trigger("rtc_set", String(newTime.toStr()));
    };

    DateTime getTime() {
        return _rtc.now();
    };

    String getName() {
        return _component;
    };


};


