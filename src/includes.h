#ifndef includes_h
#define includes_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "defaults.h"
#include "FS.h"
#include "Preferences.h"
#include "SPIFFS.h"

#ifdef ESP32
#include <WiFi.h>
//#include <ESPmDNS.h>
#include <WiFiMulti.h>
#include <esp_wifi.h>
WiFiMulti wifiMulti;
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#endif

#include <WiFiUdp.h>

#endif