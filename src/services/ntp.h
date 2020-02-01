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
    WiFiUDP _ntpUDP = WiFiUDP();
    NTPClient * _timeClient;
};

SDOS_NTP::SDOS_NTP(Debugger &debugger, EventsManager &events, AbstractRTC &rtc, WiFiManager &wifi)
    : _debugger(debugger), _events(events), _rtc(rtc), _wifi(wifi)
{
    _timeClient = new NTPClient(_ntpUDP);
};

void SDOS_NTP::setup()
{
    
};

void SDOS_NTP::loop()
{
    update();
};

void SDOS_NTP::update()
{
    bool connected = _wifi.isConnected();
    if(!connected){
        _wifi.addRequestActive();
        return;
    }
    if(_wifi.waitForConnection()){
        _debugger.Debug(_component, "Connected to Wifi. Now I'm gonna terminate.");
        _wifi.removeRequestActive();
    }
};