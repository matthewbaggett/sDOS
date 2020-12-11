#pragma once
#include "kern_inc.h"
#include "abstracts/service.hpp"
#include <NTPClient.h>

#ifndef NTP_UPDATE_INTERVAL_SECONDS
#define NTP_UPDATE_INTERVAL_SECONDS 60 * 60 * 12
#endif

class sDOS_NTP : public sDOS_Abstract_Service {

public:
    sDOS_NTP(Debugger &debugger, EventsManager &events, AbstractRTC *rtc, WiFiManager *wifi)
        : _debugger(debugger), _events(events), _rtc(rtc), _wifi(wifi) {
        _timeClient = new NTPClient(_ntpUDP, NTP_POOL, NTP_OFFSET * 3600);
        DateTime initialDateTime(1990, 6, 1);
        sDOS_NTP::_lastSuccessfulUpdateEpoch = initialDateTime.unixtime();
    };

    void setup() override {};

    void loop() override {
        update();
    };

    void update() {
        if (!needsUpdate()) {
            return;
        }

        if (!sDOS_NTP::_hasRequestedWifi) {
            _debugger.Debug(_component, "Requested wifi on.");
            _wifi->addRequestActive();
            sDOS_NTP::_hasRequestedWifi = true;
            return;
        }

        if (_wifi->isConnected()) {
            bool successfulUpdate = talkNTP();
            if (successfulUpdate) {
                _debugger.Debug(_component, "Requested wifi off.");
                _wifi->removeRequestActive();
                sDOS_NTP::_hasRequestedWifi = false;
            }
        }
    };

    String getName() override {
        return _component;
    };

    bool isActive() override {
        return needsUpdate();
    };

private:
    String _component = "NTP";
    Debugger _debugger;
    EventsManager _events;
    AbstractRTC *_rtc;
    WiFiManager *_wifi;
    WiFiUDP _ntpUDP = WiFiUDP();
    NTPClient *_timeClient;

    bool talkNTP() {
        _timeClient->begin();
        if (!_timeClient->update()) {
            return false;
        }

        _debugger.Debug(_component, "Time updated: %s", _timeClient->getFormattedTime().c_str());

        unsigned long epoch = _timeClient->getEpochTime();
        DateTime newEpoch(epoch);
        sDOS_NTP::_lastSuccessfulUpdateEpoch = newEpoch.unixtime();
        _rtc->setTime(newEpoch);
        return true;
    };

    bool needsUpdate() {
        return _rtc->getTime().unixtime() - sDOS_NTP::_lastSuccessfulUpdateEpoch > NTP_UPDATE_INTERVAL_SECONDS;
    };

    static int _lastSuccessfulUpdateEpoch;
    static bool _hasRequestedWifi;
};

int sDOS_NTP::_lastSuccessfulUpdateEpoch = 0;
bool sDOS_NTP::_hasRequestedWifi = false;

