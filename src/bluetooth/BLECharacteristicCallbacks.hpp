#ifndef BLUETOOTH_BLECHARACTERISTICCALLBACKS_HPP
#define BLUETOOTH_BLECHARACTERISTICCALLBACKS_HPP
#include "kern_inc.h"

#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLEBeacon.h"
#include "BLE2902.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

class sDOS_BLECharacteristicCallbacks: public BLECharacteristicCallbacks {
    String bleSerialBuffer;

    void onWrite(BLECharacteristic *pCharacteristic) {
      String newCharacters = pCharacteristic->getValue().c_str();
      newCharacters.replace("\r","\n");
      newCharacters.replace("\n\n","\n");
      bleSerialBuffer += newCharacters;
      if (bleSerialBuffer.length() > 0) {
        //Serial.printf("BLE Received Value: %s\n", newCharacters.c_str());
        //Serial.printf("BLE Buffer is now: %s\n", bleSerialBuffer.c_str());
        //Serial.printf("Position of newline is %d\n", bleSerialBuffer.indexOf("\n"));
        if(bleSerialBuffer.indexOf("\n") >= 0){
          String jsonSentence = bleSerialBuffer.substring(0,bleSerialBuffer.indexOf("\n"));
          bleSerialBuffer = bleSerialBuffer.substring(bleSerialBuffer.indexOf("\n") + 1);
          Serial.printf("Json Sentence: \"%s\"\n", jsonSentence.c_str());
          //Serial.printf("Remaining Buffer: \"%s\"\n", bleSerialBuffer.c_str());
          String response = processIncomingMessage(jsonSentence);
          Serial.printf("Response: \"%s\"\n", response.c_str());
          response += "\n";
          // So, we can't transmit strings longer than BLUETOOTH_MAX_PACKET_LENGTH (around 20) bytes long, so we send each section of BLUETOOTH_MAX_PACKET_LENGTH out one at a time.
          while(response.length() > 0){
            pTxCharacteristic->setValue(response.substring(0, BT_BLE_UART_MAX_PACKET).c_str());
            pTxCharacteristic->notify();
            response = response.substring(BT_BLE_UART_MAX_PACKET);
            delay(10); // Prevent BLE stack congestion
          }
        }
      }
    }

    String processIncomingMessage(String message){
      return "I recieved an incoming message";
    }
};
#endif