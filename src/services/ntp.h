#include "includes.h"

class SDOS_NTP
{
    const int NTP_UPDATE_INTERVAL_SECONDS = 60 * 60 * 12;

public:
    SDOS_NTP(Debugger &debugger, EventsManager &eventsManager, AbstractRTC *rtc, WiFiManager &wifi);
    void setup();
    void loop();
    void update();

private:
    String _component = "NTP";
    Debugger _debugger;
    EventsManager _events;
    AbstractRTC * _rtc;
    WiFiManager _wifi;
    WiFiUDP _ntpUDP = WiFiUDP();
    NTPClient * _timeClient;
    boolean talkNTP();
    boolean needsUpdate();
    static int _lastSuccessfulUpdateEpoch;
};

int SDOS_NTP::_lastSuccessfulUpdateEpoch = 0;

SDOS_NTP::SDOS_NTP(Debugger &debugger, EventsManager &events, AbstractRTC *rtc, WiFiManager &wifi)
    : _debugger(debugger), _events(events), _rtc(rtc), _wifi(wifi)
{
    _timeClient = new NTPClient(_ntpUDP,NTP_POOL, NTP_OFFSET * 3600);
    DateTime initialDateTime(1990,6,1);
    SDOS_NTP::_lastSuccessfulUpdateEpoch = initialDateTime.unixtime();
};

void SDOS_NTP::setup()
{
    _debugger.Debug(_component, "setup");   
};

void SDOS_NTP::loop()
{
    update();
};

void SDOS_NTP::update()
{
    if(!needsUpdate()){
        return;
    }
    bool connected = _wifi.isConnected();
    if(!connected){
        _wifi.addRequestActive();
        return;
    }
    if(_wifi.waitForConnection()){
        boolean successfulUpdate = talkNTP();
        if(successfulUpdate){
            _wifi.removeRequestActive();
        }
    }
};

boolean SDOS_NTP::needsUpdate(){
    bool _needsToUpdate = _rtc->getTime().unixtime() - SDOS_NTP::_lastSuccessfulUpdateEpoch > NTP_UPDATE_INTERVAL_SECONDS;
    //Serial.printf(
    //    "needs update? %s. last success %d (%d > %d)\n", 
    //    _needsToUpdate ? "Yes":"No",
    //    SDOS_NTP::_lastSuccessfulUpdateEpoch,
    //    _rtc->getTime().unixtime() - SDOS_NTP::_lastSuccessfulUpdateEpoch,
    //    NTP_UPDATE_INTERVAL_SECONDS
    //);
    if(_needsToUpdate){
        return true;
    }
    return false;
}

boolean SDOS_NTP::talkNTP(){
    _debugger.Debug(_component, "talkNTP");
    _timeClient->begin();
    if(!_timeClient->update()){
        _debugger.Debug(_component, "Time failed to update");
        return false;
    }

    _debugger.Debug(_component, "Time updated: %s", _timeClient->getFormattedTime());

    unsigned long epoch = _timeClient->getEpochTime();
    DateTime newEpoch(epoch);
    SDOS_NTP::_lastSuccessfulUpdateEpoch = newEpoch.unixtime();
    Serial.printf("Updated: %s\n", newEpoch.toStr());
    _rtc->setTime(newEpoch);

    _rtc->setAlarmInMinutes(1);
    return true;
}