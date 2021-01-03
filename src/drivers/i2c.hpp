#pragma once
#include "kern_inc.h"
#include "abstracts/driver.hpp"

class sDOS_I2C : public sDOS_Abstract_Driver {
public:
    sDOS_I2C(Debugger * debugger, EventsManager * eventsManager)
        : sDOS_Abstract_Driver(debugger, eventsManager) {
    }

    void setup() {
        this->connect();
        this->scan();
    };

    void loop() override {};

    bool isActive() override {
        return false;
    }

    static bool isConnected() {
        return sDOS_I2C::_isConnected;
    };

    void connect() {
        this->_debugger->Debug(this->_component, "I2C SDA is on port %d, SCL on port %d", I2C_SDA, I2C_SCL);
#ifdef ESP32
        if (Wire1.begin(I2C_SDA, I2C_SCL, I2C_CLOCK)) {
            this->_debugger->Debug(this->_component, "I2C Clock configured at %dkhz", I2C_CLOCK / 1000);
            this->_eventsManager->trigger("i2c_ready");
            sDOS_I2C::_isConnected = true;
        } else {
            this->_eventsManager->trigger("i2c_fail");
        }
#endif
#ifdef ESP8266
        Wire1.begin(I2C_SDA, I2C_SCL, I2C_CLOCK);
        this->_debugger->Debug(this->_component, "I2C Clock configured at %dkhz", I2C_CLOCK / 1000);
        this->_eventsManager->trigger("i2c_ready");
        sDOS_I2C::_isConnected = true;
#endif
    };

    TwoWire getWire() {
        if (!sDOS_I2C::isConnected()) {
            this->connect();
        }
        return Wire;
    };

    void scan() {
        _eventsManager->trigger("i2c_scan_begin");

        byte error, address;
        int nDevices;

        nDevices = 0;
        for (address = 1; address < 255; address++) {
            Wire1.beginTransmission(address);
            error = Wire1.endTransmission();
            if (error == 0) {
                //Serial.printf("[i2c] > i2c device found at address %#04X!\n", address);
                _eventsManager->trigger("i2c_scan_found", address);
                nDevices++;
            }
        }

        _eventsManager->trigger("i2c_scan_end");
    };

    static bool i2cDeviceExists(byte address) {
        Wire1.beginTransmission(address);
        return Wire1.endTransmission() == 0;
    };

    void write8bit(uint8_t deviceAddress, uint8_t memoryAddress, uint8_t data) {
        //this->_debugger->Debug(_component, "Writing  %s8bit%s to device %s0x%X%s location %s0x%X%s data 0x%X", COL_GREEN, COL_RESET, COL_RED, deviceAddress, COL_RESET, COL_BLUE, memoryAddress, COL_RESET, COL_YELLOW, data, COL_RESET);
        Wire1.beginTransmission(deviceAddress);
        Wire1.write(memoryAddress);
        Wire1.write(data);
        Wire1.endTransmission();

        return;

        // i2c readback, incase we're paranoid. Or debugging something fucky.
        uint8_t readback = this->read8bit(deviceAddress, memoryAddress);
        if(data != readback){
            this->_debugger->Debug(_component, "%sWriting failed, response (0x%X) doesn't match what was sent (0x%X)%s", COL_RED, data, readback, COL_RESET);
        }
    }

    uint8_t read8bit( uint8_t deviceAddress, uint8_t memoryAddress ){
        Wire1.beginTransmission(deviceAddress);
        Wire1.write(memoryAddress);
        Wire1.endTransmission(false);
        Wire1.requestFrom(deviceAddress, 1);
        uint8_t data = Wire1.read();
        //this->_debugger->Debug(_component, "Reading  %s8bit%s from device %s0x%X%s location %s0x%X%s data %s0x%X%s", COL_GREEN, COL_RESET, COL_RED, deviceAddress, COL_RESET, COL_BLUE, memoryAddress, COL_RESET, COL_YELLOW, data, COL_RESET);
        return data;
    }

    uint16_t read12bit( uint8_t deviceAddress, uint8_t memoryAddress ){
        uint16_t data = 0;
        data = (this->read8bit(deviceAddress, memoryAddress) << 4) + this->read8bit(deviceAddress, memoryAddress+1);
        //this->_debugger->Debug(_component, "Reading %s12bit%s from device %s0x%X%s location %s0x%X%s data %s0x%X%s", COL_GREEN, COL_RESET, COL_RED, deviceAddress, COL_RESET, COL_BLUE, memoryAddress, COL_RESET, COL_YELLOW, data, COL_RESET);
        return data;
    }

    uint16_t read13bit( uint8_t deviceAddress, uint8_t memoryAddress ){
        uint16_t data = 0;
        data = (this->read8bit(deviceAddress, memoryAddress) << 5) + this->read8bit(deviceAddress, memoryAddress+1);
        //this->_debugger->Debug(_component, "Reading %s13bit%s from device %s0x%X%s location %s0x%X%s data %s0x%X%s", COL_GREEN, COL_RESET, COL_RED, deviceAddress, COL_RESET, COL_BLUE, memoryAddress, COL_RESET, COL_YELLOW, data, COL_RESET);
        return data;
    }

    uint16_t read16bit( uint8_t deviceAddress, uint8_t memoryAddress )
    {
        uint16_t readData = 0;
        Wire1.beginTransmission(deviceAddress);
        Wire1.write(memoryAddress);
        Wire1.endTransmission();
        Wire1.requestFrom(deviceAddress, 2);
        for( int i = 0 ; i < 2 ; i++ )
        {
            readData <<= 8;
            readData |= Wire1.read();
        }
        return readData;
    }

    String getName() {
        return _component;
    };

protected:
    String _component = "i2c";
    static bool _isConnected;
};

bool sDOS_I2C::_isConnected = false;
