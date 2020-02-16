#ifndef SDOS_WIFI_HPP
#define SDOS_WIFI_HPP

#include "kern_inc.h"
#include "abstracts/driver.hpp"

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

public:
    WiFiManager(Debugger &debugger, FileSystem &fileSystem, EventsManager &events);

    void setup();

    void loop();

    bool hasRequests();

    void addRequestActive();

    void removeRequestActive();

    static unsigned int getRequestCount();

    bool canSleep();

    bool isConnected();

    bool isActive();

    String getName() { return _component; };

private:
    Debugger _debugger;
    FileSystem _fileSystem;
    EventsManager _events;
    const String _component = "WiFi";
    const String _fileWifiCredentials = "/wifi.json";

    void loadWifiConfigs();

    void scanWifiAps();

    enum WifiState _wifiClientState = WIFI_DISCONNECTED;
    enum WifiState _wifiClientStatePrevious = WIFI_DISCONNECTED;
    long _wifiSignalStrength = 0;
    long _wifiSignalStrengthPrevious = 0;

    void checkForStateChanges();

    long getSignalStrength();

    void powerOn();

    void powerOff();

    static unsigned int _requestsActive;

    void updateRequestedActivity();

    void updateState();

    static uint _numLoadedSSIDs;

    String getCurrentWifiMode();

    static bool _connectionDesired;
    static bool _powerOnState;
};

bool WiFiManager::_connectionDesired = false;
unsigned int WiFiManager::_requestsActive = 0;
uint WiFiManager::_numLoadedSSIDs = 0;
bool WiFiManager::_powerOnState = false;


WiFiManager::WiFiManager(Debugger &debugger, FileSystem &fileSystem, EventsManager &events)
        : _debugger(debugger), _fileSystem(fileSystem), _events(events) {}

bool WiFiManager::hasRequests() {
    return WiFiManager::_requestsActive > 0;
}

void WiFiManager::setup() {
#ifdef ESP32
    esp_wifi_deinit();
#endif
    powerOff();
    loadWifiConfigs();
    WiFiManager::_requestsActive = 0;
}

void WiFiManager::loop() {
    //_debugger.Debug(_component, "Active? %s", isActive() ? "yes" : "no");
    updateState();
    _wifiSignalStrength = WiFi.isConnected() ? getSignalStrength() : 0;
    checkForStateChanges();
    updateRequestedActivity();
}

bool WiFiManager::isConnected() {
    return WiFi.isConnected();
}

void WiFiManager::updateState() {
    if (WiFiManager::_powerOnState) {
        _wifiClientState = (wifiMulti.run() == WL_CONNECTED) ? WIFI_CONNECTED : WIFI_DISCONNECTED;
    } else {
        _wifiClientState = WIFI_DISCONNECTED;
    }
}

void WiFiManager::loadWifiConfigs() {
    delay(100);
    JsonConfigFile wifiCreds[MAX_WIFI_CREDENTIALS];
    _fileSystem.loadJsonArray(wifiCreds, _fileWifiCredentials);
    delay(100);
    for (int i = 0; i < MAX_WIFI_CREDENTIALS; i++) {
        std::map<std::string, std::string> row = wifiCreds[i].data;
        yield();
        if (!row["ssid"].empty()) {
            WiFiManager::_numLoadedSSIDs++;
            wifiMulti.addAP(row["ssid"].c_str(), row["psk"].c_str());
            _debugger.Debug(
                    _component,
                    "Loaded Wifi Credential: %s",
                    row["ssid"].c_str());
        }
        yield();
    }
}

void WiFiManager::scanWifiAps() {
    return;
    _debugger.Debug(_component, "Scanning for WiFi APs");
    int numberFound = WiFi.scanNetworks();
    if (numberFound == 0) {
        _debugger.Debug(_component, "No WiFi found");
    } else {
        for (int i = 0; i < numberFound; i++) {
            _debugger.Debug(_component, "WiFi AP Found: %s (%s)", WiFi.SSID(numberFound), WiFi.RSSI(numberFound));
        }
    }
}

void WiFiManager::checkForStateChanges() {
    if (_wifiClientStatePrevious != _wifiClientState) {
        if (_wifiClientState == WIFI_CONNECTED && _wifiClientStatePrevious == WIFI_DISCONNECTED) {
            _debugger.Debug(_component, "Connected to %s as %s", WiFi.SSID().c_str(),
                            WiFi.localIP().toString().c_str());
            _events.trigger("wifi_connect", WiFi.SSID());
            _events.trigger("wifi_ip_set", WiFi.localIP().toString());
        } else if (_wifiClientState == WIFI_DISCONNECTED && _wifiClientStatePrevious == WIFI_CONNECTED) {
            _debugger.Debug(_component, "Disconnected from Wifi.");
            _events.trigger("wifi_disconnect");
            _events.trigger("wifi_ip_unset");
        }
        _wifiClientStatePrevious = _wifiClientState;
    }
    yield();

    if (_wifiSignalStrength != _wifiSignalStrengthPrevious) {
        _wifiSignalStrengthPrevious = _wifiSignalStrength;

        //_debugger.Debug(_component, "Signal Strength changed to %d%%", _wifiSignalStrength);
        //_events.trigger("wifi_signal", _wifiSignalStrength);
    }
}

long WiFiManager::getSignalStrength() {
    long dBm = WiFi.RSSI();
    long quality;
    if (dBm <= -100)
        quality = 0;
    else if (dBm >= -50)
        quality = 100;
    else
        quality = 2 * (dBm + 100);
    return quality;
}

String WiFiManager::getCurrentWifiMode() {
#ifdef ESP32
    switch (WiFi.getMode()) {
        case WIFI_MODE_NULL:
            return "Null";
        case WIFI_MODE_STA:
            return "Station";
        case WIFI_MODE_AP:
            return "Access Point";
        case WIFI_MODE_APSTA:
            return "Access Point + Station";
        case WIFI_MODE_MAX:
            return "Max";
        default:
            return "Invalid Mode";
    }
#endif
}

void WiFiManager::powerOn() {
    if (WiFiManager::_powerOnState) {
        return;
    }
    if (WiFiManager::_numLoadedSSIDs == 0) {
        return;
    }
    WiFiManager::_powerOnState = true;
    _debugger.Debug(_component, "powerOn()");
#ifdef ESP32
    WiFi.mode(WIFI_MODE_STA);
#endif
    WiFi.persistent(false);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(true);
    _events.trigger("wifi_on");
    /*
    #if defined(ESP32) && defined(WIFI_POWER_SAVING)
    if (WIFI_POWER_SAVING == WIFI_PS_NONE){
      _debugger.Debug(_component, "WiFi power saving is disabled");
    } else if (esp_wifi_set_ps(WIFI_POWER_SAVING) == ESP_OK) {
      _debugger.Debug(_component, "Enabled WiFi power saving successfully");
      _events.trigger("wifi_powersave", F("okay"));
    } else {
      _debugger.Debug(_component, "Failed to enable WiFi power saving");
      _events.trigger("wifi_powersave", F("fail"));
    }
    #endif
    */
    scanWifiAps();
}

void WiFiManager::powerOff() {
    if (WiFiManager::_powerOnState == false) {
        return;
    }
    _debugger.Debug(_component, "powerOff()");
    WiFiManager::_powerOnState = false;
    WiFi.persistent(false);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    if (!WiFi.disconnect()) {
        _debugger.Debug(_component, "Failed calling WiFi.disconnect()");
    } else {
        _events.trigger("wifi_off");
    }
#ifdef ESP32
    WiFi.mode(WIFI_MODE_NULL);
#endif
    return;
}

unsigned int WiFiManager::getRequestCount() {
    //_debugger.Debug(_component, "wifi request count: %d, connected status %s, power state %s", _requestsActive, WiFi.isConnected() ? "Connected" : "Disconnected", WiFiManager::_wifiPowerState == true ? "on" : "off");
    return _requestsActive;
}

void WiFiManager::addRequestActive() {
    if (WiFiManager::_numLoadedSSIDs == 0) {
        // If we don't have any SSIDs loaded, we ain't connectin' to nothin'.
        return;
    }

    if (WiFiManager::_requestsActive < 0) {
        WiFiManager::_requestsActive = 0;
    }

    _debugger.Debug(_component, "Adding a wifi request");
    WiFiManager::_requestsActive++;
}

void WiFiManager::removeRequestActive() {
    if (WiFiManager::_requestsActive < 0) {
        WiFiManager::_requestsActive = 0;
    }

    if (WiFiManager::_requestsActive > 0) {
        WiFiManager::_requestsActive--;
    }
}

void WiFiManager::updateRequestedActivity() {
    if (WiFiManager::_requestsActive > 0) {
        if (getCpuFrequencyMhz() >= 80) {
            powerOn();
            return;
        }
    }
    powerOff();
}

bool WiFiManager::isActive() {
    if (WiFi.isConnected()) {
#ifdef DEBUG_WIFIMANAGER_ISACTIVE_DECISIONS
        _debugger.Debug(_component, "isActive: No. Wifi is not connected to anything");
#endif
        return true;
    }

    if (getCpuFrequencyMhz() < 80) {
#ifdef DEBUG_WIFIMANAGER_ISACTIVE_DECISIONS
        _debugger.Debug(_component, "isActive: No. CPU frequency too low");
#endif
        return false;
    }

    if (WiFiManager::_numLoadedSSIDs == 0) {
#ifdef DEBUG_WIFIMANAGER_ISACTIVE_DECISIONS
        _debugger.Debug(_component, "isActive: No. Zero SSIDs loaded");
#endif
        return false;
    }

    if (WiFiManager::_requestsActive == 0) {
#ifdef DEBUG_WIFIMANAGER_ISACTIVE_DECISIONS
        _debugger.Debug(_component, "isActive: No. Request count is 0");
#endif
        return false;
    }

#ifdef DEBUG_WIFIMANAGER_ISACTIVE_DECISIONS
    _debugger.Debug(_component, "isActive: Yes!");
#endif
    return true;
}

bool WiFiManager::canSleep() {
    if (WiFiManager::_requestsActive > 0) {
        //_debugger.Debug(_component, "canSleep: No, there are requests active: %d", WiFiManager::_requestsActive);
        return false;
    }
    if (WiFi.isConnected()) {
        //_debugger.Debug(_component, "canSleep: No, WiFi.isConnected() was true!");
        return false;
    }
    return true;
}

#endif