#include "includes.h"

class SDOS_PCF8563: public AbstractRTC
{
public:
    SDOS_PCF8563(Debugger &debugger, EventsManager &eventsManager, SDOS_I2C *i2c);
    void setup();
    void loop();
    void setAlarmInMinutes(int minutes);
    void setAlarmInSeconds(int seconds);
    void setTime(DateTime & newTime);
    DateTime getTime();

private:
    String _component = "rtc";
    static void interrupt();
    static bool hasInterruptOccured();
    static bool interruptTriggered;
    Debugger _debugger;
    EventsManager _events;
    SDOS_I2C * _i2c;
    PCF8563 _rtc;
};

bool SDOS_PCF8563::interruptTriggered = false;

SDOS_PCF8563::SDOS_PCF8563(Debugger &debugger, EventsManager & eventsManager, SDOS_I2C * i2c) 
    : _debugger(debugger), _events(eventsManager), _i2c(i2c)
    {}

void SDOS_PCF8563::setTime(DateTime & newTime){
    _rtc.adjust(newTime);
    _events.trigger("PCF8563_update", String(newTime.toStr()));
}

DateTime SDOS_PCF8563::getTime(){
    return _rtc.now();
}

void SDOS_PCF8563::setup(){
    _events.trigger(F("PCF8563_enable"));
    pinMode(PIN_INTERRUPT_PCF8563, INPUT);
    attachInterrupt(PIN_INTERRUPT_PCF8563, SDOS_PCF8563::interrupt, FALLING);
    TwoWire wire = _i2c->getWire();
    // rtc lib can't take _wire as an argument, sadly.
    _rtc.begin();
    if(_rtc.isrunning()){
        _events.trigger(F("PCF8563_ready"));
        _debugger.Debug(_component, "RTC Startup time: %s", _rtc.now().toStr());
    }else{
        _events.trigger(F("PCF8563_fail"));
        return;
    }

    setAlarmInMinutes(1);
};

void SDOS_PCF8563::setAlarmInMinutes(int minutes){
    DateTime alarm = _rtc.now();
    alarm.setminute(alarm.minute() + 1);
    _events.trigger(F("PCF8563_alarm_set"), alarm);
    _rtc.set_alarm(alarm, {AE_M, 0, 0, 0});
    _rtc.on_alarm();
}

void SDOS_PCF8563::setAlarmInSeconds(int seconds){
    DateTime alarm = _rtc.now();
    alarm.setsecond(alarm.second() + 1);
    //_events.trigger(F("PCF8563_alarm_set"), alarm);
    _rtc.set_alarm(alarm, {AE_M, 0, 0, 0});
    _rtc.on_alarm();
}

void SDOS_PCF8563::interrupt()
{
    interruptTriggered = true;
};

bool SDOS_PCF8563::hasInterruptOccured()
{
    if (interruptTriggered)
    {
        interruptTriggered = false;
        return true;
    }
    return false;
}

void SDOS_PCF8563::loop()
{
    if (SDOS_PCF8563::hasInterruptOccured())
    {
        _events.trigger(F("PCF8563_interrupt"));
        setAlarmInSeconds(5);
        //setAlarmInMinutes(1);

    }
};

