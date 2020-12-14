#pragma once
#include "kern_inc.h"
#include "abstracts/service.hpp"
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

class sDOS_OTA_Service : public sDOS_Abstract_Service {
public:
    sDOS_OTA_Service(Debugger *debugger, EventsManager *eventsManager, WiFiManager *wifi, sDOS_CPU_SCALER *cpuScaler)
        : sDOS_Abstract_Service(debugger, eventsManager),  _wifi(wifi), _cpuScaler(cpuScaler) {};

    String getName() {
        return _component;
    }

    void setup() {};

    void loop() {
        ArduinoOTA.begin();
        ArduinoOTA.handle();
    };

    bool isActive() {
        return _isActive && _wifi->isActive() && _wifi->isConnected();
    }

    void activate() {
        _debugger->Debug(_component, "activate()");
        _isActive = true;

        ArduinoOTA
        .onStart([&]() {

            _cpuScaler->onDemand(true);
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            _eventsManager->trigger(F("ota_begin"));
        });

        ArduinoOTA
        .onEnd([&]() {
            _cpuScaler->onDemand(false);
            _eventsManager->trigger(F("ota_end"));
        });

        ArduinoOTA
        .onProgress([&](unsigned int progress, unsigned int total) {
            _eventsManager->trigger(F("ota_progress"), progress);
            yield();
        });

        ArduinoOTA
        .onError([&](ota_error_t error) {

            _cpuScaler->onDemand(false);
            switch(error) {
            case OTA_AUTH_ERROR:
                _eventsManager->trigger(F("ota_err"), F("auth error"));
                break;
            case OTA_BEGIN_ERROR:
                _eventsManager->trigger(F("ota_err"), F("begin error"));
                break;
            case OTA_CONNECT_ERROR:
                _eventsManager->trigger(F("ota_err"), F("connect error"));
                break;
            case OTA_RECEIVE_ERROR:
                _eventsManager->trigger(F("ota_err"), F("receive error"));
                break;
            case OTA_END_ERROR:
                _eventsManager->trigger(F("ota_err"), F("end error"));
                break;
            }
        });

        _eventsManager->trigger(F("ota_enable"));

    }

    void deactivate() {
        _isActive = false;
        _debugger->Debug(_component, "deactivate()");
        _eventsManager->trigger(F("ota_disable"));
        ArduinoOTA.end();
    }
private:
    bool _isActive = false;
    String _component = "OTA";
    Debugger * _debugger;
    EventsManager *_events;
    WiFiManager *_wifi;
    sDOS_CPU_SCALER *_cpuScaler;
};
