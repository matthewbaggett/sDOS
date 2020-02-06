#include "kern_inc.h"
#include "abstracts/driver.hpp"

using namespace std;

#ifndef WIFI_POWER_SAVING
#define WIFI_POWER_SAVING WIFI_PS_MAX_MODEM
#endif

enum WifiState
{
  WIFI_DISCONNECTED,
  WIFI_CONNECTING,
  WIFI_CONNECTED
};

class WiFiManager : public sDOS_Abstract_Driver
{
  const unsigned int MAX_WIFI_CREDENTIALS = 10;

public:
  WiFiManager(Debugger &debugger, FileSystem &fileSystem, EventsManager &events);
  void setup();
  void loop();
  boolean hasRequests();
  void addRequestActive();
  void removeRequestActive();
  boolean waitForConnection(int timeout = 30);
  static unsigned int getRequestCount();
  boolean canSleep();
  boolean isConnected();

private:
  Debugger _debugger;
  FileSystem _fileSystem;
  EventsManager _events;
  const String _component = "WiFi";
  const String _fileWifiCredentials = "/wifi.json";
  void loadWifiConfigs();
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
  static boolean _connectionDesired;
  static boolean _powerOnState;
};

boolean WiFiManager::_connectionDesired = false;
unsigned int WiFiManager::_requestsActive = 0;
uint WiFiManager::_numLoadedSSIDs = 0;
boolean WiFiManager::_powerOnState = false;

boolean WiFiManager::hasRequests()
{
  return WiFiManager::_requestsActive > 0;
}

WiFiManager::WiFiManager(Debugger &debugger, FileSystem &fileSystem, EventsManager &events)
    : _debugger(debugger), _fileSystem(fileSystem), _events(events)
{
}

void WiFiManager::setup()
{
  powerOff();
  loadWifiConfigs();
}

void WiFiManager::loop()
{
  if(WiFiManager::getRequestCount() > 0){
    updateState();
    yield();
  }

  if (WiFi.isConnected()){
    _wifiSignalStrength = getSignalStrength();
  }else{
    _wifiSignalStrength = 0;
  }

  checkForStateChanges();
  
  updateRequestedActivity();
}

boolean WiFiManager::isConnected(){
  return WiFi.isConnected();
}

void WiFiManager::updateState()
{
  _wifiClientState = (wifiMulti.run() == WL_CONNECTED) ? WIFI_CONNECTED : WIFI_DISCONNECTED;
}

void WiFiManager::loadWifiConfigs()
{
  JsonConfigFile wifiCreds[MAX_WIFI_CREDENTIALS];
  _fileSystem.loadJsonArray(wifiCreds, _fileWifiCredentials);

  for (int i = 0; i < MAX_WIFI_CREDENTIALS; i++)
  {
    std::map<std::string, std::string> row = wifiCreds[i].data;
    yield();
    if (!row["ssid"].empty())
    {
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

void WiFiManager::checkForStateChanges()
{
  if (_wifiClientStatePrevious != _wifiClientState)
  {
    if (_wifiClientState == WIFI_CONNECTED && _wifiClientStatePrevious == WIFI_DISCONNECTED)
    {
      _debugger.Debug(_component, "Connected to %s as %s", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      _events.trigger("wifi_connect", WiFi.SSID());
      _events.trigger("wifi_ip_set", WiFi.localIP().toString());
    }
    else if (_wifiClientState == WIFI_DISCONNECTED && _wifiClientStatePrevious == WIFI_CONNECTED)
    {
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

long WiFiManager::getSignalStrength()
{
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

String WiFiManager::getCurrentWifiMode()
{
  switch (WiFi.getMode())
  {
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
}

void WiFiManager::powerOn()
{
  if(WiFiManager::_powerOnState){
    return;
  }
  WiFiManager::_powerOnState = true;

  //_debugger.Debug(_component, "powerOn()");
  WiFi.mode(WIFI_MODE_STA);

  WiFi.persistent(false);
  //WiFi.disconnect();
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(true);
  
  _events.trigger("wifi_on");
  
#ifdef WIFI_POWER_SAVING
  if (WIFI_POWER_SAVING == WIFI_PS_NONE)
  {
    _debugger.Debug(_component, "WiFi power saving is disabled");
  }
  else if (esp_wifi_set_ps(WIFI_POWER_SAVING) == ESP_OK)
  {
    _debugger.Debug(_component, "Enabled WiFi power saving successfully");
    _events.trigger("wifi_powersave", F("okay"));
  }
  else
  {
    _debugger.Debug(_component, "Failed to enable WiFi power saving");
    _events.trigger("wifi_powersave", F("fail"));
  }
#endif
}

void WiFiManager::powerOff()
{
  if(WiFiManager::_powerOnState == false){
    return;
  }

  //_debugger.Debug(_component, "powerOff()");

  WiFiManager::_powerOnState = false;

  //WiFi.mode(WIFI_MODE_NULL);

  WiFi.persistent(false);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  if(!WiFi.disconnect()){
    _debugger.Debug(_component, "Failed calling WiFi.disconnect()");
  }else{
    _events.trigger("wifi_off");
  }
  return;
}

unsigned int WiFiManager::getRequestCount()
{
  //_debugger.Debug(_component, "wifi request count: %d, connected status %s, power state %s", _requestsActive, WiFi.isConnected() ? "Connected" : "Disconnected", WiFiManager::_wifiPowerState == true ? "on" : "off");
  return _requestsActive;
}

void WiFiManager::addRequestActive()
{
  if (WiFiManager::_requestsActive >= 0)
  {
    WiFiManager::_requestsActive++;
  }
  WiFiManager::getRequestCount();
}

void WiFiManager::removeRequestActive()
{
  if (WiFiManager::_requestsActive > 0)
  {
    WiFiManager::_requestsActive--;
  }
  WiFiManager::getRequestCount();
}

void WiFiManager::updateRequestedActivity()
{
  WiFiManager::getRequestCount() > 0 ? powerOn() : powerOff();
}

boolean WiFiManager::waitForConnection(int timeout)
{
  int timeBegin = micros();
  while (true)
  {
    if (micros() > timeBegin + (timeout * 1000000))
    {
      return false;
    }

    if (WiFi.isConnected())
    {
      return true;
    }
  }
}

boolean WiFiManager::canSleep()
{
  if (WiFiManager::getRequestCount() > 0)
  {
    return false;
  }
  if (WiFi.isConnected())
  {
    return false;
  }
  return true;
}
