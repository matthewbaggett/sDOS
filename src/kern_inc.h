#ifndef includes_h
#define includes_h

#include "Arduino.h"
#include "hardware/all.h"
#include "defaults.h"
#ifdef ESP32
#include "integer.h"
#endif
#ifdef ESP8266
#include <shims/esp8266_shim.hpp>
#endif
#include "Wire.h"

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

#ifdef ESP32
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#endif

#endif