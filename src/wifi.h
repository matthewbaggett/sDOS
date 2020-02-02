#include "includes.h"
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

enum AccessPointState
{
  AP_DISABLED,
  AP_ENABLED,
  AP_BUSY
};

class WiFiManager
{
  const unsigned int MAX_WIFI_CREDENTIALS = 10;

public:
  WiFiManager(Debugger &debugger, FileSystem &fileSystem, EventsManager &events);
  void setup();
  void connect();
  void disconnect();
  void loop();
  boolean isActive();
  boolean hasRequests();
  void addRequestActive();
  void removeRequestActive();
  boolean waitForConnection(int timeout = 30);
  unsigned int getRequestCount();
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
  long _wifiSignalStrength = 89;
  long _wifiSignalStrengthPrevious = 89;
  enum AccessPointState _wifiAccessPointState = AP_DISABLED;
  void checkForStateChanges();
  long getSignalStrength();
  static boolean _isActive;
  void powerOn();
  void powerOff();
  static unsigned int _requestsActive;
  void updateRequestedActivity();
  static bool _wifiPowerState;
};

boolean WiFiManager::_isActive = false;
unsigned int WiFiManager::_requestsActive = 0;
boolean WiFiManager::_wifiPowerState = false;

boolean WiFiManager::isConnected()
{
  if (WiFiManager::_wifiPowerState)
  {
    return true;
  }
  return false;
}

boolean WiFiManager::isActive()
{
  return WiFiManager::_isActive;
}

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
  if (isActive())
  {
    uint8_t status = wifiMulti.run();
    if (status == WL_CONNECTED)
    {
      //_debugger.Debug(_component, "Connected!");
      _wifiClientState = WIFI_CONNECTED;
    }
    else
    {
      //_debugger.Debug(_component, "Not connected.");
      _wifiClientState = WIFI_DISCONNECTED;
    }
    _wifiSignalStrength = getSignalStrength();
  }
  yield();
  checkForStateChanges();
  yield();
  updateRequestedActivity();
}

void WiFiManager::loadWifiConfigs()
{
  JsonConfigFile wifiCreds[MAX_WIFI_CREDENTIALS];
  _fileSystem.loadJsonArray(wifiCreds, _fileWifiCredentials);

  for (int i = 0; i < MAX_WIFI_CREDENTIALS; i++)
  {
    std::map<std::string, std::string> row = wifiCreds[i].data;
    if (!row["ssid"].empty())
    {
      wifiMulti.addAP(row["ssid"].c_str(), row["psk"].c_str());
      _debugger.Debug(
        _component,
        "Loaded Wifi Credential: %s (%s)",
        row["ssid"].c_str(),
        row["psk"].c_str()
      );
    }
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
      _events.trigger("wifi_disconnect", WiFi.SSID());
      _events.trigger("wifi_ip_unset");
      disconnect();
    }
    _wifiClientStatePrevious = _wifiClientState;
  }
  yield();

  if (_wifiSignalStrength != _wifiSignalStrengthPrevious)
  {
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

void WiFiManager::connect()
{
  WiFiManager::_isActive = true;
  _debugger.Debug(_component, "Connect begin");
  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(true);
  powerOn();
};

void WiFiManager::powerOn()
{
  if (WiFiManager::_wifiPowerState == true)
  {
    return;
  }
  WiFiManager::_wifiPowerState = true;
  WiFi.mode(WIFI_MODE_STA);
  _events.trigger("wifi_on");

#ifdef WIFI_POWER_SAVING
  if (esp_wifi_set_ps(WIFI_POWER_SAVING) == ESP_OK)
  {
    _debugger.Debug(_component, "Enabled wifi power saving successfully");
    _events.trigger("wifi_powersave", F("okay"));
  }
  else
  {
    _debugger.Debug(_component, "Failed to enable wifi power saving");
    _events.trigger("wifi_powersave", F("fail"));
  }
#endif
}

void WiFiManager::powerOff()
{
  if (WiFiManager::_wifiPowerState == false)
  {
    return;
  }
  WiFiManager::_wifiPowerState = false;
  WiFi.disconnect();
  WiFi.mode(WIFI_MODE_NULL);
  btStop();
  _events.trigger("wifi_off");
}

void WiFiManager::disconnect()
{
  WiFiManager::_isActive = false;
  _debugger.Debug(_component, "disconnect");
};

unsigned int WiFiManager::getRequestCount()
{
  _debugger.Debug(_component, "wifi request count: %d, connected status %s, power state %s", _requestsActive, WiFi.isConnected() ? "Connected" : "Disconnected", WiFiManager::_wifiPowerState == true ? "on" : "off");
  return _requestsActive;
}

void WiFiManager::addRequestActive()
{
  if (WiFiManager::_requestsActive >= 0)
  {
    WiFiManager::_requestsActive++;
  }
  getRequestCount();
  delay(500);
}

void WiFiManager::removeRequestActive()
{
  if (WiFiManager::_requestsActive > 0)
  {
    WiFiManager::_requestsActive--;
  }
  getRequestCount();
  powerOff();
}

void WiFiManager::updateRequestedActivity()
{
  getRequestCount();
  // If cpu frequency is too low, return fast to let the scaler set the frequency.
  if (getCpuFrequencyMhz() < CPU_FREQ_MHZ)
  {
    //Serial.println("Iterate and wait for CPU scaling increase");
    if (isActive())
    {
      disconnect();
      powerOff();
    }
    return;
  }
  if (WiFiManager::_requestsActive > 0 && !isActive())
  {
    powerOn();
    connect();
  }
  else if (WiFiManager::_requestsActive == 0 && isActive())
  {
    disconnect();
    powerOff();
  }
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
    if (isConnected())
    {
      if (WiFi.isConnected())
      {
        return true;
      }
    }
  }
}