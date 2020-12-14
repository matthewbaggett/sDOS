#pragma once
#include "kern_inc.h"
#include "abstracts/driver.hpp"
#include <esp_adc_cal.h>
#include <cstdlib>
#define CHARGE_DETECT_THRESHOLD_MV 3000
#define POWER_EVENT_DELTA_TRIGGER_MV 200
class sDOS_POWER : public sDOS_Abstract_Driver {
public:

    sDOS_POWER(Debugger * debugger, EventsManager * eventsManager)
        : sDOS_Abstract_Driver(debugger, eventsManager)
    {
        esp_adc_cal_characteristics_t adc_chars;
        esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
        //Check type of calibration value used to characterize ADC
        if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
            _debugger->Debug(_component, "eFuse Vref:%u mV", adc_chars.vref);
            _vref = adc_chars.vref;
        } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
            _debugger->Debug(_component, "Two Point --> coeff_a:%umV coeff_b:%umV", adc_chars.coeff_a, adc_chars.coeff_b);
        } else {
            _debugger->Debug(_component, "Default Vref: 1100mV");
        }

#ifdef POWER_MONITOR_VBATT
        debugger->Debug(_component, "Enabled monitoring VBATT on pin #%d", POWER_MONITOR_VBATT);
        pinMode(POWER_MONITOR_VBATT, INPUT);
        //analogSetPinAttenuation(POWER_MONITOR_VBATT, ADC_0db);
#endif

#ifdef POWER_MONITOR_VBUS
        debugger->Debug(_component, "Enabled monitoring VBUS on pin #%d", POWER_MONITOR_VBUS);
        pinMode(POWER_MONITOR_VBUS, INPUT);
        //analogSetPinAttenuation(POWER_MONITOR_VBUS, ADC_0db);
#endif

#ifdef POWER_MONITOR_CHARGE_STATE
        debugger->Debug(_component, "Enabled monitoring charging state on pin #%d", POWER_MONITOR_CHARGE_STATE);
        pinMode(POWER_MONITOR_CHARGE_STATE, INPUT_PULLUP);
        attachInterrupt(POWER_MONITOR_CHARGE_STATE, [] {
            _chargingInterruptTriggered = true;
        }, CHANGE);
        if(digitalRead(POWER_MONITOR_CHARGE_STATE) == LOW) {
            _chargingInterruptTriggered = true;
        }
#endif
        _eventsManager->trigger(F("power_ready"));

    };

    float voltageRead(int adcPin) {
        uint16_t v = analogRead(adcPin);
        return ((float)v / 4095.0) * 2.0 * 3.3 * (_vref / 1000.0);
    }

    int voltageReadMillivolts(int adcPin) {
        return static_cast<int>((voltageRead(adcPin)*1000));
    };

    void loop() override {
#ifdef POWER_MONITOR_CHARGE_STATE
        if (sDOS_POWER::_chargingInterruptTriggered) {
            sDOS_POWER::_chargingInterruptTriggered = false;
            sDOS_POWER::_isCharging = digitalRead(POWER_MONITOR_CHARGE_STATE) == LOW;

            _debugger->Debug(_component, "Charging status changed to %s",
                            sDOS_POWER::_isCharging ? "charging" : "discharging");
            _eventsManager->trigger(F("power_charging"), sDOS_POWER::_isCharging ? "yes" : "no");
        }
#endif
#ifdef POWER_MONITOR_VBATT
        sDOS_POWER::_mon_mv_vbatt = voltageReadMillivolts(POWER_MONITOR_VBATT);
        if (sDOS_POWER::_mon_mv_vbatt != sDOS_POWER::_mon_mv_vbatt_previous  && std::abs(sDOS_POWER::_mon_mv_vbatt - sDOS_POWER::_mon_mv_vbatt_previous ) > POWER_EVENT_DELTA_TRIGGER_MV) {
            _eventsManager->trigger(F("power_vbatt_mv"), sDOS_POWER::_mon_mv_vbatt);
            sDOS_POWER::_mon_mv_vbatt_previous = sDOS_POWER::_mon_mv_vbatt;
        }
#endif
#ifdef POWER_MONITOR_VBUS
        sDOS_POWER::_mon_mv_vbus = voltageReadMillivolts(POWER_MONITOR_VBUS);
        if (sDOS_POWER::_mon_mv_vbus != sDOS_POWER::_mon_mv_vbus_previous && std::abs(sDOS_POWER::_mon_mv_vbus - sDOS_POWER::_mon_mv_vbus_previous ) > POWER_EVENT_DELTA_TRIGGER_MV) {
            _eventsManager->trigger(F("power_vbus_mv"), sDOS_POWER::_mon_mv_vbus);
            //_debugger->Debug(_component, "vbus prev: %d vbus now: %d, threshold: %d", sDOS_POWER::_mon_mv_vbus_previous, sDOS_POWER::_mon_mv_vbus, CHARGE_DETECT_THRESHOLD_MV);
            if(sDOS_POWER::_mon_mv_vbus_previous > CHARGE_DETECT_THRESHOLD_MV && sDOS_POWER::_mon_mv_vbus <= CHARGE_DETECT_THRESHOLD_MV) {
                _eventsManager->trigger(F("power_state"),F("unplugged"));
            } else if(sDOS_POWER::_mon_mv_vbus_previous <= CHARGE_DETECT_THRESHOLD_MV && sDOS_POWER::_mon_mv_vbus > CHARGE_DETECT_THRESHOLD_MV) {
                _eventsManager->trigger(F("power_state"),F("charging"));
            }
            sDOS_POWER::_mon_mv_vbus_previous = sDOS_POWER::_mon_mv_vbus;
        }
#endif
    };

    static bool isCharging() {
        return sDOS_POWER::_isCharging;
    };

#ifdef POWER_MONITOR_VBATT
    static int getVbattMv() {
        return sDOS_POWER::_mon_mv_vbatt;
    };
#endif
#ifdef POWER_MONITOR_VBUS
    static int getVbusMv() {
        return sDOS_POWER::_mon_mv_vbus;
    };
#endif
    String getName() override {
        return _component;
    };
private:
#ifdef POWER_MONITOR_VBATT
    static int _mon_mv_vbatt;
    static int _mon_mv_vbatt_previous;
#endif
#ifdef POWER_MONITOR_VBUS
    static int _mon_mv_vbus;
    static int _mon_mv_vbus_previous;
#endif
    Debugger * _debugger;
    EventsManager * _eventsManager;
    static bool _isCharging;
    static bool _chargingInterruptTriggered;
    String _component = "PWRMGMT";
    int _vref = 1100;
};

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

bool Debugger::isPowerCharging() {
    return sDOS_POWER::isCharging();
}
double Debugger::getBatteryVolts() {
    return double(sDOS_POWER::getVbattMv()) / 1000;
};