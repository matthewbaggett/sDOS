#pragma once
#include "../kern_inc.h"
#include "../abstracts/driver.hpp"

using namespace std;

#ifndef WIFI_POWER_SAVING
#define WIFI_POWER_SAVING WIFI_PS_MAX_MODEM
#endif

enum WifiState {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED
};

class WiFiManager : public sDOS_Abstract_Driver {
    const unsigned int MAX_WIFI_CREDENTIALS = 10;

protected:
    FileSystem *_fileSystem;
    const String _component = "WiFi";
    const String _fileWifiCredentials = "/wifi.json";
    enum WifiState _wifiClientState = WIFI_DISCONNECTED;
    enum WifiState _wifiClientStatePrevious = WIFI_DISCONNECTED;
    long _wifiSignalStrength = 0;
    long _wifiSignalStrengthPrevious = 0;
    static unsigned int _requestsActive;
    static uint _numLoadedSSIDs;
    static bool _powerOnState;

public:
    WiFiManager(Debugger * debugger, FileSystem * fileSystem, EventsManager * eventsManager)
        : sDOS_Abstract_Driver(debugger, eventsManager), _fileSystem(fileSystem) {
        //debugger->Debug(_component, "Construct");
    };

    void setup() override {
#ifdef ESP32
        esp_wifi_deinit();
#endif
        this->powerOff();
        this->loadWifiConfigs();
        WiFiManager::_requestsActive = 0;
    };

    void loop() override {
        //this->_debugger->Debug(_component, "Active? %s", isActive() ? "yes" : "no");
        updateState();
        _wifiSignalStrength = WiFi.isConnected() ? getSignalStrength() : 0;
        checkForStateChanges();
        updateRequestedActivity();
    };

    void addRequestActive() {
        if (WiFiManager::_numLoadedSSIDs == 0) return;
        this->_debugger->Debug(_component, "Requests Active: %d", WiFiManager::_requestsActive);
        WiFiManager::_requestsActive = WiFiManager::_requestsActive <= 0 ? 1 : WiFiManager::_requestsActive + 1;
    }

    void removeRequestActive() {
        this->_debugger->Debug(_component, "Requests Active: %d", WiFiManager::_requestsActive);
        WiFiManager::_requestsActive = WiFiManager::_requestsActive <= 0 ? 0 : WiFiManager::_requestsActive - 1;
    }

    static unsigned int getRequestCount() {
        return _requestsActive;
    };

    static bool canSleep() {
        return !(WiFiManager::_requestsActive > 0 || WiFi.isConnected());
    }

    static bool isConnected() {
        return WiFi.isConnected();
    };

    bool isActive() override {
        return !(getCpuFrequencyMhz() < 80 && WiFiManager::_numLoadedSSIDs == 0
                 && WiFiManager::_requestsActive == 0 && !WiFi.isConnected());
    };

    String getName() {
        return _component;
    };

protected:
    void loadWifiConfigs() {
        delay(100);
        JsonConfigFile wifiCreds[MAX_WIFI_CREDENTIALS];
        this->_fileSystem->loadJsonArray(wifiCreds, this->_fileWifiCredentials);
        delay(100);
        for (int i = 0; i < MAX_WIFI_CREDENTIALS; i++) {
            std::map<std::string, std::string> row = wifiCreds[i].data;
            yield();
            if (!row["ssid"].empty()) {
                WiFiManager::_numLoadedSSIDs++;
                wifiMulti.addAP(row["ssid"].c_str(), row["psk"].c_str());
                this->_debugger->Debug(
                    _component,
                    "Loaded Wifi Credential: %s",
                    row["ssid"].c_str());
            }
            yield();
        }
    };

    void checkForStateChanges() {
        if (_wifiClientStatePrevious != _wifiClientState) {
            if (_wifiClientState == WIFI_CONNECTED && _wifiClientStatePrevious == WIFI_DISCONNECTED) {
                this->_debugger->Debug(_component, "Connected to %s as %s", WiFi.SSID().c_str(),
                                WiFi.localIP().toString().c_str());
                this->_eventsManager->trigger("wifi_connect", WiFi.SSID());
                this->_eventsManager->trigger("wifi_ip_set", WiFi.localIP().toString());
            } else if (_wifiClientState == WIFI_DISCONNECTED && _wifiClientStatePrevious == WIFI_CONNECTED) {
                this->_debugger->Debug(_component, "Disconnected from Wifi.");
                this->_eventsManager->trigger("wifi_disconnect");
                this->_eventsManager->trigger("wifi_ip_unset");
            }
            _wifiClientStatePrevious = _wifiClientState;
        }
        yield();

        if (_wifiSignalStrength != _wifiSignalStrengthPrevious) {
            _wifiSignalStrengthPrevious = _wifiSignalStrength;

            //this->_debugger->Debug(_component, "Signal Strength changed to %d%%", _wifiSignalStrength);
            //this->_eventsManager->trigger("wifi_signal", _wifiSignalStrength);
        }
    };

    static long getSignalStrength() {
        long dBm = WiFi.RSSI();
        long quality;
        if (dBm <= -100)
            quality = 0;
        else if (dBm >= -50)
            quality = 100;
        else
            quality = 2 * (dBm + 100);
        return quality;
    };

    void powerOn() {
        if (WiFiManager::_powerOnState) {
            return;
        }
        if (WiFiManager::_numLoadedSSIDs == 0) {
            return;
        }
        WiFiManager::_powerOnState = true;
        this->_debugger->Debug(_component, "powerOn()");
#ifdef ESP32
        WiFi.mode(WIFI_MODE_STA);
#endif
        WiFi.persistent(false);
        WiFi.setAutoConnect(false);
        WiFi.setAutoReconnect(true);
        this->_eventsManager->trigger("wifi_on");
        return;
#if defined(ESP32) && defined(WIFI_POWER_SAVING)
        if (WIFI_POWER_SAVING == WIFI_PS_NONE) {
            this->_debugger->Debug(_component, "WiFi power saving is disabled");
        } else if (esp_wifi_set_ps(WIFI_POWER_SAVING) == ESP_OK) {
            this->_debugger->Debug(_component, "Enabled WiFi power saving successfully");
            this->_eventsManager->trigger("wifi_powersave", F("okay"));
        } else {
            this->_debugger->Debug(_component, "Failed to enable WiFi power saving");
            this->_eventsManager->trigger("wifi_powersave", F("fail"));
        }
#endif
    };

    void powerOff() {
        if (!WiFiManager::_powerOnState) {
            return;
        }
        this->_debugger->Debug(_component, "powerOff()");
        WiFiManager::_powerOnState = false;
        WiFi.persistent(false);
        WiFi.setAutoConnect(false);
        WiFi.setAutoReconnect(false);
        if (!WiFi.disconnect()) {
            this->_debugger->Debug(_component, "Failed calling WiFi.disconnect()");
        } else {
            this->_eventsManager->trigger("wifi_off");
        }
#ifdef ESP32
        WiFi.mode(WIFI_MODE_NULL);
#endif
    };


    void updateRequestedActivity() {
        if (WiFiManager::_requestsActive > 0) {
            if (getCpuFrequencyMhz() >= 80) {
                powerOn();
                return;
            }
        }
        powerOff();
    };

    void updateState() {
        if (WiFiManager::_powerOnState) {
            _wifiClientState = (wifiMulti.run() == WL_CONNECTED) ? WIFI_CONNECTED : WIFI_DISCONNECTED;
        } else {
            _wifiClientState = WIFI_DISCONNECTED;
        }
    };

};

unsigned int WiFiManager::_requestsActive = 0;
uint WiFiManager::_numLoadedSSIDs = 0;
bool WiFiManager::_powerOnState = false;

bool Debugger::isWifiPoweredOn() {
    return WiFi.isConnected();
}

