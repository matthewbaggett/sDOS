#ifndef PCF8563_H
#define PCF8563_H
#include "kernel_inc.h"
#include "RTClib.h"
#include "abstracts/rtc.h"

class SDOS_PCF8563: public AbstractRTC
{
public:
    SDOS_PCF8563(Debugger & debugger, EventsManager & eventsManager, SDOS_I2C & i2c){
        _debugger = debugger;
        _eventsManager = eventsManager;
        _i2c = i2c;
    };
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
    Debugger &_debugger;
    EventsManager &_eventsManager;
    SDOS_I2C &_i2c;
    PCF8563 _rtc;
};

bool SDOS_PCF8563::interruptTriggered = false;

void SDOS_PCF8563::setTime(DateTime & newTime){
    _rtc.adjust(newTime);
    _eventsManager.trigger("PCF8563_update", String(newTime.toStr()));
}

DateTime SDOS_PCF8563::getTime(){
    return _rtc.now();
}

void SDOS_PCF8563::setup(){
    _eventsManager.trigger(F("PCF8563_enable"));
    pinMode(PIN_INTERRUPT_PCF8563, INPUT);
    attachInterrupt(PIN_INTERRUPT_PCF8563, SDOS_PCF8563::interrupt, FALLING);
    gpio_wakeup_enable(PIN_INTERRUPT_PCF8563, GPIO_INTR_LOW_LEVEL);
    TwoWire wire = _i2c->getWire();
    // rtc lib can't take _wire as an argument, sadly.
    _rtc.begin();
    if(_rtc.isrunning()){
        _eventsManager.trigger(F("PCF8563_ready"));
        _debugger.Debug(_component, "RTC Startup time: %s", _rtc.now().toStr());
    }else{
        _eventsManager.trigger(F("PCF8563_fail"));
        return;
    }

    setAlarmInMinutes(1);
};

void SDOS_PCF8563::setAlarmInMinutes(int minutes){
    DateTime alarm = _rtc.now();
    alarm.setminute(alarm.minute() + minutes);
    _eventsManager.trigger(F("PCF8563_alarm_set"), alarm);
    _rtc.set_alarm(alarm, {AE_M, 0, 0, 0});
    _rtc.on_alarm();
}

void SDOS_PCF8563::setAlarmInSeconds(int seconds){
    DateTime alarm = _rtc.now();
    alarm.setsecond(alarm.second() + seconds);
    _eventsManager.trigger(F("PCF8563_alarm_set"), alarm);
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
        _eventsManager.trigger(F("PCF8563_interrupt"));
        setAlarmInMinutes(1);

    }
};

#endif