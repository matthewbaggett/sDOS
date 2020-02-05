#ifndef BLUETOOTH_HPP
#define BLUETOOTH_HPP
#include "kernel_inc.h"

using namespace std;
#define BLUETOOTH_MAX_PACKET_LENGTH 20
/*
BLEServer *pServer = NULL;
BLEAdvertising *pAdvertising;
BLECharacteristic *pCharacteristic;
BLECharacteristic *pTxCharacteristic;
BLECharacteristic *pRxCharacteristic;

enum BluetoothState
{
  BLUETOOTH_DISCONNECTED,
  BLUETOOTH_CONNECTING,
  BLUETOOTH_CONNECTED
};

class BluetoothDriverBLEServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer * pServer);
    void onDisconnect(BLEServer * pServer);
};

class BluetoothDriverBLECallbacks: public BLECharacteristicCallbacks {
    public: 
      void onWrite(BLECharacteristic *pCharacteristic);
      String processIncomingMessage(String incomingMessage);
    
    private: 
      String bleSerialBuffer;
};

class BluetoothDriver : public sDOS_Abstract_Driver
{
  public:
      BluetoothDriver(Debugger &debugger, EventsManager &events) : _debugger(debugger), _events(events) {};
      void setup() override;
      void loop() override;
      void powerOn();
      void powerOff();
      void setHostname(String newHostname);
      static void setHasConnectedDevice(boolean hasConnectedDevice);
      static boolean getHasConnectedDevice();
      static const BLEUUID UART_SERVICE_UUID;

  private:
      String _component = "BLE";
      Debugger _debugger;
      EventsManager _events;
      static boolean _powerState;
      static String _bleHostname;
      static boolean _hasConnectedDevice;
};




void BluetoothDriverBLEServerCallbacks::onConnect(BLEServer *pServer)
{
    Serial.println("DongsOS_BLEServerCallbacks::onConnect");
    BluetoothDriver::setHasConnectedDevice(true);
}

void BluetoothDriverBLEServerCallbacks::onDisconnect(BLEServer *pServer)
{
    Serial.println("DongsOS_BLEServerCallbacks::onDisconnect");
    BluetoothDriver::setHasConnectedDevice(false);
}

String BluetoothDriverBLECallbacks::processIncomingMessage(String incomingMessage)
{
    return "Response goes here";
}

void BluetoothDriverBLECallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
    String newCharacters = pCharacteristic->getValue().c_str();
    newCharacters.replace("\r", "\n");
    newCharacters.replace("\n\n", "\n");
    bleSerialBuffer += newCharacters;
    if (bleSerialBuffer.length() > 0)
    {
        //Serial.printf("BLE Received Value: %s\n", newCharacters.c_str());
        //Serial.printf("BLE Buffer is now: %s\n", bleSerialBuffer.c_str());
        //Serial.printf("Position of newline is %d\n", bleSerialBuffer.indexOf("\n"));
        if (bleSerialBuffer.indexOf("\n") >= 0)
        {
            String incoming = bleSerialBuffer.substring(0, bleSerialBuffer.indexOf("\n"));
            bleSerialBuffer = bleSerialBuffer.substring(bleSerialBuffer.indexOf("\n") + 1);
            Serial.printf("Incoming Sentence: \"%s\"\n", incoming.c_str());
            //Serial.printf("Remaining Buffer: \"%s\"\n", bleSerialBuffer.c_str());
            String response = processIncomingMessage(incoming);
            Serial.printf("Response: \"%s\"\n", response);
            response += "\n";
            // So, we can't transmit strings longer than BLUETOOTH_MAX_PACKET_LENGTH (around 20) bytes long, so we send each section of BLUETOOTH_MAX_PACKET_LENGTH out one at a time.
            while (response.length() > 0)
            {
                pTxCharacteristic->setValue(response.substring(0, BLUETOOTH_MAX_PACKET_LENGTH).c_str());
                pTxCharacteristic->notify();
                response = response.substring(BLUETOOTH_MAX_PACKET_LENGTH);
                delay(10); // Prevent BLE stack congestion
            }
        }
    }
}

boolean BluetoothDriver::_powerState = false;
String BluetoothDriver::_bleHostname = F("sDOS Device");
boolean BluetoothDriver::_hasConnectedDevice = false;

void BluetoothDriver::setup()
{
    BluetoothDriver::UART_SERVICE_UUID.fromString("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
}

void BluetoothDriver::loop()
{
}

void BluetoothDriver::powerOn()
{
    if (BluetoothDriver::_powerState == true)
    {
        return;
    }
    BluetoothDriver::_powerState = true;
    BLEDevice::init(_bleHostname.c_str());

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new BluetoothDriverBLEServerCallbacks());
    BLEService *pService = pServer->createService(BluetoothDriver::UART_SERVICE_UUID);

    pTxCharacteristic = pService->createCharacteristic(
        BluetoothDriver::UART_SERVICE_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pTxCharacteristic->addDescriptor(new BLE2902());

    pRxCharacteristic = pService->createCharacteristic(
        BluetoothDriver::UART_SERVICE_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );

    pRxCharacteristic->setCallbacks(new BluetoothDriverBLECallbacks());

    // Start advertising
    pService->start();
    pServer->getAdvertising()->start();
    _debugger.Debug(_component, "BLE Server running");
    _events.trigger("ble_on");
}

void BluetoothDriver::powerOff()
{
    if (BluetoothDriver::_powerState == false)
    {
        return;
    }
    BluetoothDriver::_powerState = false;
    BLEDevice::deinit();
    _events.trigger("ble_off");
}

void BluetoothDriver::setHostname(String newHostname)
{
    BluetoothDriver::_bleHostname = newHostname;
    // If bluetooth is on, restart it.
    if (BluetoothDriver::_powerState)
    {
        powerOff();
        powerOn();
    }
}

void BluetoothDriver::setHasConnectedDevice(boolean hasConnectedDevice)
{
    BluetoothDriver::_hasConnectedDevice = hasConnectedDevice;
}

boolean BluetoothDriver::getHasConnectedDevice()
{
    return BluetoothDriver::_hasConnectedDevice;
}
*/
#endif