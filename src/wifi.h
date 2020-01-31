#include "includes.h"

#ifndef WIFI_POWER_SAVING
#define WIFI_POWER_SAVING WIFI_PS_MAX_MODEM
#endif
enum WifiState { 
  WIFI_DISCONNECTED,
  WIFI_CONNECTING,
  WIFI_CONNECTED
};

enum AccessPointState {
  AP_DISABLED,
  AP_ENABLED,
  AP_BUSY
};

class WiFiManager{
    public:
        WiFiManager(Debugger& debugger, FileSystem& fileSystem);
        void connect();
        void disconnect();
        void loop();
    private:
        Debugger _debugger;
        FileSystem _fileSystem;
        String _component = "WiFi";
        String _fileWifiCredentials = "/wifi.json";
        void loadWifiConfigs();
        enum WifiState _wifiClientState = WIFI_DISCONNECTED;
        enum WifiState _wifiClientStatePrevious = WIFI_DISCONNECTED;
        long _wifiSignalStrength = 89;
        long _wifiSignalStrengthPrevious = 89;
        enum AccessPointState _wifiAccessPointState = AP_DISABLED;
        void checkForStateChanges();
        long getSignalStrength();
};


WiFiManager::WiFiManager(Debugger& debugger, FileSystem& fileSystem) 
            : _debugger(debugger), _fileSystem(fileSystem)
{
    
}

void WiFiManager::loop(){
  uint8_t status = wifiMulti.run();
  if(status == WL_CONNECTED){
    _wifiClientState = WIFI_CONNECTED;
  }else{
    Serial.println("Wifi not connected");
    _wifiClientState = WIFI_DISCONNECTED;
  }
  _wifiSignalStrength = getSignalStrength();

  checkForStateChanges();
}

void WiFiManager::loadWifiConfigs() {
    JsonArray wifiConfigs = _fileSystem.loadJsonArray(_fileWifiCredentials);
    if(wifiConfigs.success()){
        for (auto wifiConfig : wifiConfigs){
            String ssid = wifiConfig["ssid"];
            String psk = wifiConfig["psk"];
            _debugger.Debug(_component, "Stored Wifi credentials found: %s (%s)", ssid.c_str(), psk.c_str());
            wifiMulti.addAP(ssid.c_str(), psk.c_str());
        }
    }
}

void WiFiManager::checkForStateChanges(){
    if(_wifiClientStatePrevious != _wifiClientState){
        if(_wifiClientState == WIFI_CONNECTED && _wifiClientStatePrevious == WIFI_DISCONNECTED){
            _debugger.Debug(_component, "Connected to %s as %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
        }else if(_wifiClientState == WIFI_DISCONNECTED && _wifiClientStatePrevious == WIFI_CONNECTED){
            _debugger.Debug(_component, "Disconnected from Wifi.");
        }
        _wifiClientStatePrevious = _wifiClientState;
    }

  if(_wifiSignalStrength != _wifiSignalStrengthPrevious){
    _wifiSignalStrengthPrevious = _wifiSignalStrength;
    //_debugger.Debug(_component, "Signal Strength changed to %d%%", _wifiSignalStrength);
  }
}

long WiFiManager::getSignalStrength() { 
  long dBm = WiFi.RSSI();
  long quality;
  if(dBm <= -100)
    quality = 0;
  else if(dBm >= -50)
    quality = 100;
  else
    quality = 2 * (dBm + 100);
  return quality;
}
void WiFiManager::connect() {
    _debugger.Debug(_component, "Connect begin");
    WiFi.persistent(false);
    WiFi.disconnect();
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_AP_STA);
    #ifdef WIFI_POWER_SAVING
        if(esp_wifi_set_ps(WIFI_POWER_SAVING) == ESP_OK){
            _debugger.Debug(_component, "Enabled wifi power saving successfully");
        }else{
            _debugger.Debug(_component, "Failed to enable wifi power saving");
        }
    #endif;
    loadWifiConfigs();
};

void WiFiManager::disconnect() {
    _debugger.Debug(_component, "disconnect");
};