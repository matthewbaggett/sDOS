#pragma once
#include "kern_inc.h"
#include "abstracts/service.hpp"

class sDOS_CPU_SCALER : public sDOS_Abstract_Service {

public:
    sDOS_CPU_SCALER(Debugger &debugger, EventsManager &eventsManager, WiFiManager *wifi, BluetoothManager *bluetooth)
        : _debugger(debugger), _events(eventsManager), _wifi(wifi), _bluetooth(bluetooth) {};

    void setup() override {};

    void loop() override {
        updateFrequency();
    };

    void setCPUMhz(uint32_t targetFreq) {
        uint32_t currentFrequency = getCpuFrequencyMhz();

        if(getCpuFrequencyMhz() == targetFreq) return;

        setCpuFrequencyMhz(targetFreq);

        if( targetFreq > currentFrequency ){
            _events.trigger(F("cpu_scaling_increase"));
        }else{
            _events.trigger(F("cpu_scaling_decrease"));
        }
    };

    uint32_t updateFrequency() {
#ifdef CPU_FREQ_MHZ
        uint32_t targetFreq = CPU_FREQ_MHZ;
#ifdef CPU_FREQ_MHZ_NORADIO
        if (isSlowPossible()) {
            targetFreq = CPU_FREQ_MHZ_NORADIO;
        }
#endif
        setCPUMhz(targetFreq);
#endif
        uint32_t currentFreq = getCpuFrequencyMhz();
        if (currentFreq <= CPU_FREQ_MHZ_NORADIO) {
            TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
            TIMERG0.wdt_feed = 1;
            TIMERG0.wdt_wprotect = 0;
        }
        return currentFreq;
    };

    String getName() override { return _component; };

    void onDemand(bool onDemandStateDesired){
        _isOnDemand = onDemandStateDesired;
        if (_isOnDemand) {
            _onDemandPreviousFrequency = getCpuFrequencyMhz();
            setCPUMhz(CPU_FREQ_MHZ_ONDEMAND);
        } else {
            setCPUMhz(_onDemandPreviousFrequency);
        }
        yield();
    };

private:
    String _component = "SCALER";
    Debugger _debugger;
    EventsManager _events;
    WiFiManager *_wifi;
    BluetoothManager *_bluetooth;

    bool isSlowPossible(){
#ifdef DEBUG_CPU_SCALER_DECISIONS
        _debugger.Debug(
        _component,
        "Is Slow Possible? Wifi: %s%s%s. Wifi Requests: %s%d%s. BlueTooth: %s%s%s",
        _wifi->canSleep() ? COL_GREEN : COL_RED,
        _wifi->canSleep() ? F("possible") : F("NOT POSSIBLE"),
        COL_RESET,
        _wifi->getRequestCount() > 0 ? COL_RED : COL_GREEN,
        _wifi->getRequestCount(),
        COL_RESET,
        _bluetooth->canSleep() ? COL_GREEN : COL_RED,
        _bluetooth->canSleep() ? F("possible") : F("NOT POSSIBLE"),
        COL_RESET
    );
#endif
        return  _wifi->canSleep()
                && !(bluetoothState != BT_DISABLED)
                && BluetoothManager::getRequestCount() == 0
                && !_isOnDemand;
    };

    bool _isOnDemand = false;
    uint32_t _onDemandPreviousFrequency = CPU_FREQ_MHZ;
};