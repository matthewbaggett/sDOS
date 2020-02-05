#ifndef includes_h
#define includes_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "defaults.h"
#include "integer.h"

// i2c related
#include "Wire.h"

// Bluetooth related
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLEBeacon.h"
#include "BLE2902.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"



#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#include "esp32-hal-gpio.h"

#include <SparkFunMPU9250-DMP.h>

#include "abstracts/driver.h"
#include "abstracts/service.h"
#include "abstracts/rtc.h"
#include "abstracts/accellerometer.h"

#include "hardware/all.h"

#endif