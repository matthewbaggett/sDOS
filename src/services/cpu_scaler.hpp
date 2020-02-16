#ifndef SDOS_SERVICES_CPUSCALER_HPP
#define SDOS_SERVICES_CPUSCALER_HPP
#include "kern_inc.h"
#include "abstracts/service.hpp"

class sDOS_CPU_SCALER : public sDOS_Abstract_Service
{

public:
    sDOS_CPU_SCALER(Debugger &debugger, EventsManager &eventsManager, WiFiManager *wifi, BluetoothManager *bluetooth);
    void setup();
    void loop();
    uint32_t updateFrequency();
    String getName(){ return _component; };
    void onDemand(bool onDemandStateDesired);

private:
    String _component = "SCALER";
    Debugger _debugger;
    EventsManager _events;
    WiFiManager * _wifi;
    BluetoothManager * _bluetooth;
    bool isSlowPossible();
    bool _isOnDemand = false;
    uint32_t _onDemandPreviousFrequency = CPU_FREQ_MHZ;
};

sDOS_CPU_SCALER::sDOS_CPU_SCALER(Debugger &debugger, EventsManager &events, WiFiManager *wifi, BluetoothManager *bluetooth)
    : _debugger(debugger), _events(events), _wifi(wifi), _bluetooth(bluetooth)
    {};

void sDOS_CPU_SCALER::setup(){};

void sDOS_CPU_SCALER::onDemand(bool ondemandStateDesired){
    _isOnDemand = ondemandStateDesired;
    if(_isOnDemand){
        _onDemandPreviousFrequency = getCpuFrequencyMhz();
        setCpuFrequencyMhz(CPU_FREQ_MHZ_ONDEMAND);
    }else{
        setCpuFrequencyMhz(_onDemandPreviousFrequency);
    }
    yield();
};

bool sDOS_CPU_SCALER::isSlowPossible()
{
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
    return _wifi->canSleep() && _bluetooth->canSleep() && _isOnDemand == false;
}

void sDOS_CPU_SCALER::loop()
{
    sDOS_CPU_SCALER::updateFrequency();
}

uint32_t sDOS_CPU_SCALER::updateFrequency() {
    uint32_t currentFreq;
#ifdef CPU_FREQ_MHZ
    uint32_t targetFreq = CPU_FREQ_MHZ;
#ifdef CPU_FREQ_MHZ_NORADIO
    if (isSlowPossible())
    {
        targetFreq = CPU_FREQ_MHZ_NORADIO;
    }
#endif
    currentFreq = getCpuFrequencyMhz();
    if (currentFreq != targetFreq)
    {
        setCpuFrequencyMhz(targetFreq);
        _debugger.Debug(_component, "CPU frequency changed from %dMhz to %dMhz", currentFreq, getCpuFrequencyMhz());
        _events.trigger("cpu_freq_mhz", getCpuFrequencyMhz());
    }
#endif
    currentFreq = getCpuFrequencyMhz();
    if (currentFreq <= CPU_FREQ_MHZ_NORADIO)
    {
        TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
        TIMERG0.wdt_feed = 1;
        TIMERG0.wdt_wprotect = 0;
    }
    return currentFreq;   
}
#endif