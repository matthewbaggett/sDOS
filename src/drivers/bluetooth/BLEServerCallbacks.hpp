#ifndef BLUETOOTH_BLESERVERCALLBACKS_H
#define BLUETOOTH_BLESERVERCALLBACKS_H

#include "kern_inc.h"

#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLEBeacon.h"
#include "BLE2902.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

class sDOS_BLEServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        bluetoothState = BluetoothState::BT_CONNECTED;
    };

    void onDisconnect(BLEServer *pServer) {
        bluetoothState = BluetoothState::BT_ENABLED;
    }
};

#endif