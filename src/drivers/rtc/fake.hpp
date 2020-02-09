#include "kern_inc.h"
#include "abstracts/rtc.hpp"
#include <RTClib.h>

class sDOS_FAKE_RTC: public AbstractRTC
{
public:
    sDOS_FAKE_RTC(Debugger &debugger, EventsManager &eventsManager);
    void setup();
    void loop();
    void setAlarmInMinutes(int minutes);
    void setAlarmInSeconds(int seconds);
    void setTime(DateTime & newTime);
    DateTime getTime();
    String getName(){ return _component; };

private:
    String _component = "fakertc";
    static void interrupt();
    static bool hasInterruptOccured();
    static bool interruptTriggered;
    Debugger _debugger;
    EventsManager _events;
    RTC_Millis _rtc;
};

bool sDOS_FAKE_RTC::interruptTriggered = false;

sDOS_FAKE_RTC::sDOS_FAKE_RTC(Debugger &debugger, EventsManager & eventsManager) 
    : _debugger(debugger), _events(eventsManager)
    {}

void sDOS_FAKE_RTC::setTime(DateTime & newTime){
    _rtc.adjust(newTime);
    _events.trigger("rtc_set", String(newTime.toStr()));
}

DateTime sDOS_FAKE_RTC::getTime(){
    return _rtc.now();
}

void sDOS_FAKE_RTC::setup(){
    DateTime buildTime = DateTime(__DATE__,__TIME__);
    _events.trigger(F("rtc_enable"));
    _rtc.begin(buildTime);
    _events.trigger(F("rtc_ready"));
    _debugger.Debug(_component, "RTC Startup time: %s", _rtc.now().toStr());
};

void sDOS_FAKE_RTC::setAlarmInMinutes(int minutes){

};

void sDOS_FAKE_RTC::setAlarmInSeconds(int seconds){

};

void sDOS_FAKE_RTC::loop()
{
    
};
