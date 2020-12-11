#include "kern_inc.h"
#include "rtc.hpp"
#include <RTClib.h>

class sDOS_FAKE_RTC : public AbstractRTC {
private:
    String _component = "fakertc";
    Debugger _debugger;
    EventsManager _events;
    RTC_Millis _rtc;
public:
    sDOS_FAKE_RTC(Debugger &debugger, EventsManager &eventsManager)
        : _debugger(debugger), _events(eventsManager) {};

    void setup() {
        DateTime buildTime = DateTime(__DATE__, __TIME__);
        _events.trigger(F("rtc_enable"));
        _rtc.begin(buildTime);
        _events.trigger(F("rtc_ready"));
        _debugger.Debug(_component, "RTC Startup time: %s", _rtc.now().toStr());
    };

    void loop() {};

    void setAlarmInMinutes(int minutes) {};

    void setAlarmInSeconds(int seconds) {};

    void setTime(DateTime &newTime) {
        _rtc.adjust(newTime);
        _events.trigger("rtc_set", String(newTime.toStr()));
    };

    DateTime getTime() {
        return _rtc.now();
    };

    String getName() {
        return _component;
    };


};


