#include "kern_inc.h"
#include "abstracts/driver.hpp"

class sDOS_POWER : public sDOS_Abstract_Driver
{
public:
    sDOS_POWER(Debugger &debugger, EventsManager &eventsManager);
    void setup();
    void loop();
    //static TwoWire wire;
    static bool isCharging();
    #ifdef POWER_MONITOR_VBATT
    static int getVbattMv();
    #endif
    #ifdef POWER_MONITOR_VBUS
    static int getVbusMv();
    #endif
    String getName(){ return _component; };

private:
#ifdef POWER_MONITOR_CHARGE_STATE
    static void interrupt();
#endif
#ifdef POWER_MONITOR_VBATT
    static int _mon_mv_vbatt;
    static int _mon_mv_vbatt_previous;
#endif
#ifdef POWER_MONITOR_VBUS
    static int _mon_mv_vbus;
    static int _mon_mv_vbus_previous;
#endif
    Debugger _debugger;
    EventsManager _events;
    static bool _isCharging;
    static bool _chargingInterruptTriggered;
    String _component = "PWRMGMT";
};

//TwoWire sDOS_POWER::wire = Wire;
bool sDOS_POWER::_isCharging = false;
bool sDOS_POWER::_chargingInterruptTriggered = false;
#ifdef POWER_MONITOR_VBATT
    int sDOS_POWER::_mon_mv_vbatt = 0;
    int sDOS_POWER::_mon_mv_vbatt_previous = 0;
#endif
#ifdef POWER_MONITOR_VBUS
    int sDOS_POWER::_mon_mv_vbus = 0;
    int sDOS_POWER::_mon_mv_vbus_previous = 0;
#endif

sDOS_POWER::sDOS_POWER(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger), _events(eventsManager){};

void sDOS_POWER::setup()
{
#ifdef POWER_MONITOR_VBATT
    pinMode(POWER_MONITOR_VBATT, INPUT);
    analogSetPinAttenuation(POWER_MONITOR_VBATT,ADC_0db);
#endif

#ifdef POWER_MONITOR_VBUS
    pinMode(POWER_MONITOR_VBUS, INPUT);
    analogSetPinAttenuation(POWER_MONITOR_VBUS,ADC_0db);
#endif

#ifdef POWER_MONITOR_CHARGE_STATE
    pinMode(POWER_MONITOR_CHARGE_STATE, INPUT);
    attachInterrupt(POWER_MONITOR_CHARGE_STATE, sDOS_POWER::interrupt, CHANGE);
    _isCharging = digitalRead(POWER_MONITOR_CHARGE_STATE);
#endif
    _events.trigger("POWER_ready");
};

#ifdef POWER_MONITOR_CHARGE_STATE
void sDOS_POWER::interrupt()
{
    _chargingInterruptTriggered = true;
};
#endif
void sDOS_POWER::loop()
{
#ifdef POWER_MONITOR_CHARGE_STATE
    if (sDOS_POWER::_chargingInterruptTriggered)
    {
        sDOS_POWER::_chargingInterruptTriggered = false;
        _debugger.Debug(_component, "Charging status changed to %s", digitalRead(POWER_MONITOR_CHARGE_STATE) ? "charging" : "discharging");
        // @todo trigger event
    }
#endif
#ifdef POWER_MONITOR_VBATT
    sDOS_POWER::_mon_mv_vbatt = analogRead(POWER_MONITOR_VBATT) * BATTERY_MAGIC_MULTIPLIER;
    if(!(round(sDOS_POWER::_mon_mv_vbatt_previous/100) == round(sDOS_POWER::_mon_mv_vbatt/100))){
        _debugger.Debug(_component, "VBATT %dMv", sDOS_POWER::_mon_mv_vbatt);
        sDOS_POWER::_mon_mv_vbatt_previous = sDOS_POWER::_mon_mv_vbatt;
        _events.trigger(F("power_vbatt_mv"), sDOS_POWER::_mon_mv_vbatt);
    }
#endif
#ifdef POWER_MONITOR_VBUS
    sDOS_POWER::_mon_mv_vbus = (float) analogRead(POWER_MONITOR_VBUS) * BATTERY_MAGIC_MULTIPLIER;
    if(!(round(sDOS_POWER::_mon_mv_vbus_previous/100) == round(sDOS_POWER::_mon_mv_vbus/100))){
        _debugger.Debug(_component, "VBUS %dMv", sDOS_POWER::_mon_mv_vbus);
        sDOS_POWER::_mon_mv_vbus_previous = sDOS_POWER::_mon_mv_vbus;
        _events.trigger(F("power_vbus_mv"), sDOS_POWER::_mon_mv_vbus);
    }
#endif
};

bool sDOS_POWER::isCharging()
{
    return sDOS_POWER::_isCharging;
};

#ifdef POWER_MONITOR_VBATT
int sDOS_POWER::getVbattMv()
{
    return sDOS_POWER::_mon_mv_vbatt;
}
#endif

#ifdef POWER_MONITOR_VBUS
int sDOS_POWER::getVbusMv()
{
    return sDOS_POWER::_mon_mv_vbus;
}
#endif
