#pragma once
#include "kern_inc.h"
#include "abstracts/service.hpp"
#include <NTPClient.h>

#ifndef NTP_UPDATE_INTERVAL_SECONDS
#define NTP_UPDATE_INTERVAL_SECONDS 60 * 60 * 12
#endif

class sDOS_NTP : public sDOS_Abstract_Service {

public:
    sDOS_NTP(Debugger * debugger, EventsManager * events, AbstractRTC *rtc, WiFiManager *wifi)
        : sDOS_Abstract_Service(debugger, events), _rtc(rtc), _wifi(wifi) {
    }

    void setup() override {

        Serial.println("sDOS_NTP::setup() A");
        Serial.flush();
        this->_timeClient = new NTPClient(_ntpUDP, NTP_POOL, NTP_OFFSET * 3600);
        Serial.println("sDOS_NTP::setup() B");
        Serial.flush();
        DateTime initialDateTime(1990, 6, 1);
        Serial.println("sDOS_NTP::setup() C");
        Serial.flush();
        this->_lastSuccessfulUpdateEpoch = initialDateTime.unixtime();
        Serial.println("sDOS_NTP::setup() D");
        Serial.flush();
    };

    void loop() override {
        Serial.println("sDOS_NTP::loop()");

        if (!this->needsUpdate()) {
            return;
        }

        if (!sDOS_NTP::_hasRequestedWifi) {
            _debugger->Debug(_component, "Requested wifi on.");
            _wifi->addRequestActive();
            sDOS_NTP::_hasRequestedWifi = true;
            return;
        }

        if (_wifi->isConnected()) {
            bool successfulUpdate = talkNTP();
            if (successfulUpdate) {
                _debugger->Debug(_component, "Requested wifi off.");
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

protected:
    String _component = "NTP";
    AbstractRTC *_rtc;
    WiFiManager *_wifi;
    WiFiUDP _ntpUDP = WiFiUDP();
    NTPClient *_timeClient;
    int _lastSuccessfulUpdateEpoch;
    static bool _hasRequestedWifi;

    bool talkNTP() {
        _timeClient->begin();
        if (!_timeClient->update()) {
            return false;
        }

        _debugger->Debug(_component, "Time updated: %s", _timeClient->getFormattedTime().c_str());

        unsigned long epoch = _timeClient->getEpochTime();
        DateTime newEpoch(epoch);
        this->_lastSuccessfulUpdateEpoch = newEpoch.unixtime();
        _rtc->setTime(newEpoch);
        return true;
    };

    bool needsUpdate() {
        return _rtc->getTime().unixtime() - this->_lastSuccessfulUpdateEpoch > NTP_UPDATE_INTERVAL_SECONDS;
    };
};

bool sDOS_NTP::_hasRequestedWifi = false;

