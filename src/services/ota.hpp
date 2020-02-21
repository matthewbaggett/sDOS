#pragma once
#include "kern_inc.h"
#include "abstracts/service.hpp"
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

class sDOS_OTA_Service : public sDOS_Abstract_Service {
public:
    sDOS_OTA_Service(Debugger &debugger, EventsManager &events, WiFiManager *wifi, sDOS_CPU_SCALER *cpuScaler)
        : _debugger(debugger), _events(events),  _wifi(wifi), _cpuScaler(cpuScaler) {};

    String getName() { return _component; }

    void setup(){};

    void loop(){
        ArduinoOTA.begin();
        ArduinoOTA.handle();
    };

    bool isActive(){ return _isActive && _wifi->isActive() && _wifi->isConnected(); }

    void activate(){
        _debugger.Debug(_component, "activate()");
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
                    _events.trigger("ota_begin");
                });

        ArduinoOTA
                .onEnd([&]() {
                    _cpuScaler->onDemand(false);
                    _events.trigger("ota_end");
                });

        ArduinoOTA
                .onProgress([&](unsigned int progress, unsigned int total) {
                    _events.trigger("ota_progress", progress);
                    yield();
                });

        ArduinoOTA
                .onError([&](ota_error_t error) {

                    _cpuScaler->onDemand(false);
                    switch(error){
                        case OTA_AUTH_ERROR:
                            _events.trigger("ota_err", "auth error");
                            break;
                        case OTA_BEGIN_ERROR:
                            _events.trigger("ota_err", "begin error");
                            break;
                        case OTA_CONNECT_ERROR:
                            _events.trigger("ota_err", "connect error");
                            break;
                        case OTA_RECEIVE_ERROR:
                            _events.trigger("ota_err", "receive error");
                            break;
                        case OTA_END_ERROR:
                            _events.trigger("ota_err", "end error");
                            break;
                    }
                    _events.trigger("ota_err", error);
                });

        _events.trigger("ota_enable");

    }

    void deactivate(){
        _isActive = false;
        _debugger.Debug(_component, "deactivate()");
        _events.trigger("ota_disable");
        ArduinoOTA.end();
    }
private:
    bool _isActive = false;
    String _component = "OTA";
    Debugger _debugger;
    EventsManager _events;
    WiFiManager *_wifi;
    sDOS_CPU_SCALER *_cpuScaler;
};
