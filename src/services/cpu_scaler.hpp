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

private:
    String _component = "SCALER";
    Debugger _debugger;
    EventsManager _events;
    WiFiManager * _wifi;
    BluetoothManager * _bluetooth;
    boolean isSlowPossible();

};

sDOS_CPU_SCALER::sDOS_CPU_SCALER(Debugger &debugger, EventsManager &events, WiFiManager *wifi, BluetoothManager *bluetooth)
    : _debugger(debugger), _events(events), _wifi(wifi), _bluetooth(bluetooth)
    {};

void sDOS_CPU_SCALER::setup()
{
};

boolean sDOS_CPU_SCALER::isSlowPossible()
{
    /*_debugger.Debug(
        _component, 
        "is slow possible? wifi: %s req: %d. bluetooth: %s",
        _wifi->canSleep() ? "yes" : "no", 
        _wifi->getRequestCount(),
        _bluetooth->canSleep() ? "yes" : "no"
     );*/
    return _wifi->canSleep() && _bluetooth->canSleep();
}

void sDOS_CPU_SCALER::loop()
{
    sDOS_CPU_SCALER::updateFrequency();
}

uint32_t sDOS_CPU_SCALER::updateFrequency() {
    uint32_t currentFreq;
#ifdef CPU_FREQ_MHZ
    uint32_t targetFreq = CPU_FREQ_MHZ;
    /*
    _debugger.Debug(
        _component,
        "_wifi.isActive = %s, _wifi.getRequestCount = %d, WiFi.isConnected = %s, cansleep? %s",
        driver_WiFi->isActive() ? "yes" : "no",
        driver_WiFi->getRequestCount(), 
        WiFi.isConnected() ? "yes" : "no",
        driver_WiFi->canSleep() ? "yes" : "no"
    );
    */
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
