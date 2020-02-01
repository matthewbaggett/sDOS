#include "includes.h"

class SDOS_NTP
{
public:
    SDOS_NTP(Debugger &debugger, EventsManager &eventsManager, AbstractRTC &rtc, WiFiManager &wifi);
    void setup();
    void loop();
    void update();

private:
    String _component = "NTP";
    Debugger _debugger;
    EventsManager _events;
    AbstractRTC _rtc;
    WiFiManager _wifi;
};

SDOS_NTP::SDOS_NTP(Debugger &debugger, EventsManager &events, AbstractRTC &rtc, WiFiManager &wifi)
    : _debugger(debugger), _events(events), _rtc(rtc), _wifi(wifi)
{};

void SDOS_NTP::setup()
{
    update();
};

void SDOS_NTP::loop()
{
    
};

void SDOS_NTP::update()
{
    _wifi.addRequestActive();
    return;
    if(_wifi.waitForConnection()){
        _debugger.Debug(_component, "Connected to Wifi. Now I'm gonna terminate.");
    }else{
        _debugger.Debug(_component, "Failed to connect to Wifi. Now I'm gonna terminate.");
    }
    //_wifi.removeRequestActive();
};