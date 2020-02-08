#include "kern_inc.h"
#include "abstracts/service.hpp"
#include <NTPClient.h>

#ifndef NTP_UPDATE_INTERVAL_SECONDS
#define NTP_UPDATE_INTERVAL_SECONDS 60 * 60 * 12
#endif

class SDOS_NTP : public sDOS_Abstract_Service
{

public:
    SDOS_NTP(Debugger &debugger, EventsManager &eventsManager, AbstractRTC *rtc, WiFiManager *wifi);
    void setup();
    void loop();
    void update();
    String getName(){ return _component; };
    
private:
    String _component = "NTP";
    Debugger _debugger;
    EventsManager _events;
    AbstractRTC *_rtc;
    WiFiManager *_wifi;
    WiFiUDP _ntpUDP = WiFiUDP();
    NTPClient *_timeClient;
    boolean talkNTP();
    boolean needsUpdate();
    static int _lastSuccessfulUpdateEpoch;
    static boolean _hasRequestedWifi;
};

int SDOS_NTP::_lastSuccessfulUpdateEpoch = 0;
boolean SDOS_NTP::_hasRequestedWifi = false;

SDOS_NTP::SDOS_NTP(Debugger &debugger, EventsManager &events, AbstractRTC *rtc, WiFiManager *wifi)
    : _debugger(debugger), _events(events), _rtc(rtc), _wifi(wifi)
{
    _timeClient = new NTPClient(_ntpUDP, NTP_POOL, NTP_OFFSET * 3600);
    DateTime initialDateTime(1990, 6, 1);
    SDOS_NTP::_lastSuccessfulUpdateEpoch = initialDateTime.unixtime();
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
    if (!needsUpdate())
    {
        return;
    }
    
    if (!SDOS_NTP::_hasRequestedWifi)
    {
        _wifi->addRequestActive();
        SDOS_NTP::_hasRequestedWifi = true;
        _debugger.Debug(_component, "Requested wifi on.");
        return;
    }
    
    if (_wifi->isConnected())
    {
        boolean successfulUpdate = talkNTP();
        if (successfulUpdate)
        {
            _debugger.Debug(_component, "Requested wifi off.");
            _wifi->removeRequestActive();
            SDOS_NTP::_hasRequestedWifi = false;
        }
    }
};

boolean SDOS_NTP::needsUpdate()
{
    bool _needsToUpdate = _rtc->getTime().unixtime() - SDOS_NTP::_lastSuccessfulUpdateEpoch > NTP_UPDATE_INTERVAL_SECONDS;
    /*_debugger.Debug(
        _component,
        "needs update? %s. last success %d (%d > %d)",
        _needsToUpdate ? "Yes":"No",
        SDOS_NTP::_lastSuccessfulUpdateEpoch,
        _rtc->getTime().unixtime() - SDOS_NTP::_lastSuccessfulUpdateEpoch,
        NTP_UPDATE_INTERVAL_SECONDS
    );*/
    return _needsToUpdate;
}

boolean SDOS_NTP::talkNTP()
{
    _timeClient->begin();
    if (!_timeClient->update())
    {
        return false;
    }

    _debugger.Debug(_component, "Time updated: %s", _timeClient->getFormattedTime());

    unsigned long epoch = _timeClient->getEpochTime();
    DateTime newEpoch(epoch);
    SDOS_NTP::_lastSuccessfulUpdateEpoch = newEpoch.unixtime();
    //Serial.printf("Updated: %s\n", newEpoch.toStr());
    _rtc->setTime(newEpoch);

    _rtc->setAlarmInSeconds(5);
    return true;
}