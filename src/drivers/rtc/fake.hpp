#include "kern_inc.h"
#include "abstracts/rtc.hpp"

class SDOS_FAKE_RTC: public AbstractRTC
{
public:
    SDOS_FAKE_RTC(Debugger &debugger, EventsManager &eventsManager);
    void setup();
    void loop();
    void setAlarmInMinutes(int minutes);
    void setAlarmInSeconds(int seconds);
    void setTime(DateTime & newTime);
    DateTime getTime();
    String getName(){ return _component; };

private:
    String _component = "rtc";
    Debugger _debugger;
    EventsManager _events;
};

SDOS_FAKE_RTC::SDOS_FAKE_RTC(Debugger &debugger, EventsManager & eventsManager) 
    : _debugger(debugger), _events(eventsManager)
    {}

void SDOS_FAKE_RTC::setTime(DateTime & newTime){
    _events.trigger("rtc_set", String(newTime.toStr()));
}

DateTime SDOS_FAKE_RTC::getTime(){
    DateTime now = DateTime(__DATE__,__TIME__);
    return now;
}

void SDOS_FAKE_RTC::setup(){
    _events.trigger(F("rtc_enable"));
    _events.trigger(F("rtc_ready"));
    setAlarmInMinutes(1);
};

void SDOS_FAKE_RTC::setAlarmInMinutes(int minutes){
    DateTime alarm = DateTime(__DATE__, __TIME__);
    alarm.setminute(alarm.minute() + minutes);
    _events.trigger(F("rtc_alarm_set"), alarm);
};

void SDOS_FAKE_RTC::setAlarmInSeconds(int seconds){
    DateTime alarm = DateTime(__DATE__, __TIME__);
    alarm.setsecond(alarm.second() + seconds);
    _events.trigger(F("rtc_alarm_set"), alarm);
};

void SDOS_FAKE_RTC::loop()
{
};

