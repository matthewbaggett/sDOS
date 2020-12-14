#pragma once

#include <kern_inc.h>
#include "abstracts/driver.hpp"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEBeacon.h>
#include <BLE2902.h>
#include <BLE2902.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>
#include <debugger.hpp>
#include <events.hpp>

enum BluetoothState {
    BT_DISABLED,
    BT_ENABLED,
    BT_CONNECTED
};
BLEServer *pServer = nullptr;
BLEAdvertising *pAdvertising;
BLECharacteristic *pCharacteristic;
BLECharacteristic *pTxCharacteristic;
BLECharacteristic *pRxCharacteristic;
BluetoothState bluetoothState;

#include "bluetooth/BLECharacteristicCallbacks.hpp"
#include "bluetooth/BLEServerCallbacks.hpp"

using namespace std;

class BluetoothManager : public sDOS_Abstract_Driver {
private:
    Debugger * _debugger;
    EventsManager * _events;
    String _component = "Bluetoof";
    String bleHostname = "sDOS device";
    static unsigned int _requestsActive;

public:
    static const unsigned int MAX_BLE_PACKET = 20;
    const BLEUUID UART_SERVICE_UUID = BLEUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    const BLEUUID UART_CHARACTERISTIC_RX_UUID = BLEUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
    const BLEUUID UART_CHARACTERISTIC_TX_UUID = BLEUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

    BluetoothManager(Debugger * debugger, EventsManager * events)
        : _debugger(debugger), _events(events) {};

    void setup() override {
        BluetoothManager::_requestsActive = 0;
        powerOff();
        _debugger->addHandler(&BluetoothManager::sendMessage);
#if defined(BT_BLE_UART_DEBUG) && BT_BLE_UART_DEBUG == true
        addRequested();
#endif
    };

    void loop() override {
        if (BluetoothManager::getRequestCount() > 0) {
            if (getCpuFrequencyMhz() >= CPU_FREQ_MHZ_MIN_RADIO) {
                if (!BLEDevice::getInitialized()) {
                    powerOn();
                } else {
                    if (bluetoothState == BluetoothState::BT_ENABLED || bluetoothState == BluetoothState::BT_CONNECTED) {
                        pServer->startAdvertising();
                    }
                }
            }
        }
    };

    void setHostname(const String& newHostname) {
        _debugger->Debug(_component, "Changed BLE Hostname to %s", newHostname.c_str());
        bleHostname = newHostname;
        if ((bluetoothState != BT_DISABLED)) {
            _debugger->Debug(_component, "Restarting BLE");
            powerOff();
            powerOn();
        }
    };

    static void sendMessage(String message) {
        if (bluetoothState == BluetoothState::BT_CONNECTED) {
            while (message.length() > 0) {
                if (esp_bt_controller_is_sleeping()) {
                    esp_bt_controller_wakeup_request();
                    while (esp_bt_controller_is_sleeping()) {
                        delay(1);
                    }
                }
                pTxCharacteristic->setValue(message.substring(0, BluetoothManager::MAX_BLE_PACKET).c_str());
                pTxCharacteristic->notify();
                message = message.substring(BluetoothManager::MAX_BLE_PACKET);
                delay(10);
            }
        }
    };

    static unsigned int getRequestCount() {
        return BluetoothManager::_requestsActive;
    };

    static void addRequest() {
        BluetoothManager::_requestsActive++;
    };

    static void removeRequest() {
        if(BluetoothManager::_requestsActive > 0) {
            BluetoothManager::_requestsActive--;
        }
    };

    static bool isPoweredOn() {
        return (bluetoothState != BluetoothState::BT_DISABLED);
    };

    String getName() override {
        return _component;
    };

    bool canSleep() {
        return BluetoothManager::_requestsActive == 0;
    }

private:
    void powerOn() {
        if (bluetoothState != BluetoothState::BT_DISABLED) {
            return;
        }
        bluetoothState = BluetoothState::BT_ENABLED;
        _debugger->Debug(_component, "BLE powerOn");

        BLEDevice::init(bleHostname.c_str());
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new sDOS_BLEServerCallbacks());
        BLEService *pService = pServer->createService(UART_SERVICE_UUID);

        pTxCharacteristic = pService->createCharacteristic(
                                UART_CHARACTERISTIC_TX_UUID,
                                BLECharacteristic::PROPERTY_NOTIFY
                            );

        pTxCharacteristic->addDescriptor(new BLE2902());

        pRxCharacteristic = pService->createCharacteristic(
                                UART_CHARACTERISTIC_RX_UUID,
                                BLECharacteristic::PROPERTY_WRITE
                            );

        pRxCharacteristic->setCallbacks(new sDOS_BLECharacteristicCallbacks());

        if (ESP_OK != esp_bt_sleep_enable()) {
            log_e("Failed to enter ESP BT sleep");
        }

        // Start advertising
        pService->start();
        pServer->getAdvertising()->start();

        // Release some RAM used for BT Classic that we dont use
        if(ESP_OK != esp_bt_mem_release(ESP_BT_MODE_CLASSIC_BT)) {
            _debugger->Debug(_component, "Cannot free bt memory");
        }
        if(ESP_OK != esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT)) {
            _debugger->Debug(_component, "Cannot free bt controller memory");
        }

        _events->trigger("bluetooth_on");
    };

    void powerOff() {
        if (bluetoothState == BluetoothState::BT_DISABLED) {
            return;
        }
        bluetoothState = BluetoothState::BT_DISABLED;

        _debugger->Debug(_component, "powerOff()");
        _events->trigger("bluetooth_off");
    };
};
unsigned int BluetoothManager::_requestsActive = 0;

bool Debugger::isBluetoothPoweredOn() {
    return BluetoothManager::isPoweredOn();
}