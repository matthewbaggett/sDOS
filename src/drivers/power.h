#include "includes.h"

class SDOS_POWER
{
public:
    SDOS_POWER(Debugger &debugger, EventsManager &eventsManager);
    void setup();
    void loop();
    //static TwoWire wire;
    static bool isCharging();

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
    String _component = "PowerMGMT";
};

//TwoWire SDOS_POWER::wire = Wire;
bool SDOS_POWER::_isCharging = false;
bool SDOS_POWER::_chargingInterruptTriggered = false;
#ifdef POWER_MONITOR_VBATT
    int SDOS_POWER::_mon_mv_vbatt = 0;
    int SDOS_POWER::_mon_mv_vbatt_previous = 0;
#endif
#ifdef POWER_MONITOR_VBUS
    int SDOS_POWER::_mon_mv_vbus = 0;
    int SDOS_POWER::_mon_mv_vbus_previous = 0;
#endif

SDOS_POWER::SDOS_POWER(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger), _events(eventsManager){};

void SDOS_POWER::setup()
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
    attachInterrupt(POWER_MONITOR_CHARGE_STATE, SDOS_POWER::interrupt, CHANGE);
#endif
    _events.trigger("POWER_ready");
};

#ifdef POWER_MONITOR_CHARGE_STATE
void SDOS_POWER::interrupt()
{
    _chargingInterruptTriggered = true;
};
#endif
void SDOS_POWER::loop()
{
#ifdef POWER_MONITOR_CHARGE_STATE
    if (SDOS_POWER::_chargingInterruptTriggered)
    {
        SDOS_POWER::_chargingInterruptTriggered = true;
        _debugger.Debug(_component, "Charging status changed to %s", digitalRead(POWER_MONITOR_CHARGE_STATE) ? "charging" : "discharging");
        // @todo trigger event
    }
#endif
#ifdef POWER_MONITOR_VBATT
    SDOS_POWER::_mon_mv_vbatt = analogRead(POWER_MONITOR_VBATT) * BATTERY_MAGIC_MULTIPLIER;
    if(!(round(SDOS_POWER::_mon_mv_vbatt_previous/100) == round(SDOS_POWER::_mon_mv_vbatt/100))){
        _debugger.Debug(_component, "VBATT %dMv", SDOS_POWER::_mon_mv_vbatt);
        SDOS_POWER::_mon_mv_vbatt_previous = SDOS_POWER::_mon_mv_vbatt;
        _events.trigger(F("power_vbatt_mv"), SDOS_POWER::_mon_mv_vbatt);
    }
#endif
#ifdef POWER_MONITOR_VBUS
    SDOS_POWER::_mon_mv_vbus = (float) analogRead(POWER_MONITOR_VBUS) * BATTERY_MAGIC_MULTIPLIER;
    if(!(round(SDOS_POWER::_mon_mv_vbus_previous/100) == round(SDOS_POWER::_mon_mv_vbus/100))){
        _debugger.Debug(_component, "VBUS %dMv", SDOS_POWER::_mon_mv_vbus);
        SDOS_POWER::_mon_mv_vbus_previous = SDOS_POWER::_mon_mv_vbus;
        _events.trigger(F("power_vbus_mv"), SDOS_POWER::_mon_mv_vbus);
    }
#endif
};

bool SDOS_POWER::isCharging()
{
    return SDOS_POWER::_isCharging;
};
