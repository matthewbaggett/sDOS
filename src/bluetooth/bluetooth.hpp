#ifndef SDOS_BLUETOOTH_HPP
#define SDOS_BLUETOOTH_HPP
#include "kern_inc.h"
#include "abstracts/driver.hpp"

#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLEBeacon.h"
#include "BLE2902.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

enum BluetoothState
{
  BT_DISABLED,
  BT_ENABLED,
  BT_CONNECTED
};
BLEServer         * pServer = NULL;
BLEAdvertising    * pAdvertising;
BLECharacteristic * pCharacteristic;
BLECharacteristic * pTxCharacteristic;
BLECharacteristic * pRxCharacteristic;
BluetoothState bluetoothState;

#include "bluetooth/BLECharacteristicCallbacks.hpp"
#include "bluetooth/BLEServerCallbacks.hpp"

using namespace std;

class BluetoothManager : public sDOS_Abstract_Driver
{
  public:
    static const unsigned int MAX_BLE_PACKET = 20;
    const BLEUUID UART_SERVICE_UUID = BLEUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    const BLEUUID UART_CHARACTERISTIC_RX_UUID = BLEUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
    const BLEUUID UART_CHARACTERISTIC_TX_UUID = BLEUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
    BluetoothManager(Debugger &debugger, EventsManager &events);
    void setup();
    void loop();
    static bool canSleep();
    static bool isPoweredOn();
    void debugMessage(String message);
    void setHostname(String newHostname);
    void addRequested();
    void removeRequested();
    static void sendMessage(String message);
    String getName() { return _component; };

  private:
    Debugger _debugger;
    EventsManager _events;
    String _component = "Bluetoof";
    void powerOn();
    void powerOff();
    String bleHostname = "sDOS device";
    static int _requested;
};

int BluetoothManager::_requested = 0;

BluetoothManager::BluetoothManager(Debugger &debugger, EventsManager &events)
    : _debugger(debugger), _events(events)
{
}

void BluetoothManager::addRequested()
{
  BluetoothManager::_requested++;
}

void BluetoothManager::removeRequested()
{
  BluetoothManager::_requested--;
  if(BluetoothManager::_requested <= 0){
    BluetoothManager::_requested = 0;
  }
}

void BluetoothManager::debugMessage(String message)
{
  _debugger.Debug(_component, message.c_str());
}

void BluetoothManager::setup()
{
  powerOff();
  _debugger.addHandler(&BluetoothManager::sendMessage);
#if defined(BT_BLE_UART_DEBUG) && BT_BLE_UART_DEBUG == true
  addRequested();
#endif
}

void BluetoothManager::loop()
{
  if(BluetoothManager::_requested > 0) {
    if(getCpuFrequencyMhz() >= CPU_FREQ_MHZ_MIN_RADIO) {
        if(!BLEDevice::getInitialized()){
          powerOn();
        }else{
          if(bluetoothState == BluetoothState::BT_CONNECTED){
            pServer->startAdvertising();
          }
        }
    }
  }
}

void BluetoothManager::powerOn()
{
  if(bluetoothState != BluetoothState::BT_DISABLED){
    return;
  }
  bluetoothState = BluetoothState::BT_ENABLED;
  _debugger.Debug(_component, "powerOn()");

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
  
  if(ESP_OK != esp_bt_sleep_enable()){
    log_e("Failed to enter ESP BT sleep");
  }
  
  // Start advertising
  pService->start();
  pServer->getAdvertising()->start();
  _events.trigger("bluetooth_on");
}

void BluetoothManager::powerOff()
{
  if(bluetoothState == BluetoothState::BT_DISABLED){
    return;
  }
  bluetoothState = BluetoothState::BT_DISABLED;

  _debugger.Debug(_component, "powerOff()");
  _events.trigger("bluetooth_off");
}

bool BluetoothManager::isPoweredOn()
{
  return (bluetoothState != BluetoothState::BT_DISABLED);
}

bool BluetoothManager::canSleep()
{
  return ! BluetoothManager::isPoweredOn() && _requested == 0;
}

void BluetoothManager::setHostname(String newHostname)
{
  _debugger.Debug(_component, "Changed BLE Hostname to %s", newHostname.c_str());
  bleHostname = newHostname;
  if(BluetoothManager::isPoweredOn()){
    _debugger.Debug(_component, "Restarting BLE");
    powerOff();
    powerOn();
  }
}

void BluetoothManager::sendMessage(String message){
  if(bluetoothState == BluetoothState::BT_CONNECTED){
    while(message.length() > 0){
      if(esp_bt_controller_is_sleeping()){
        esp_bt_controller_wakeup_request();
        while(esp_bt_controller_is_sleeping()){
         delay(1);
        }
      }
      pTxCharacteristic->setValue(message.substring(0, BluetoothManager::MAX_BLE_PACKET).c_str());
      pTxCharacteristic->notify();
      message = message.substring(BluetoothManager::MAX_BLE_PACKET);
      delay(10);
    }
  }
}

#endif