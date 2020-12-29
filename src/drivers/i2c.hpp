#pragma once
#include "kern_inc.h"
#include "abstracts/driver.hpp"

class sDOS_I2C : public sDOS_Abstract_Driver {
public:
    sDOS_I2C(Debugger * debugger, EventsManager * eventsManager)
        : sDOS_Abstract_Driver(debugger, eventsManager) {}

    void setup() {
        Serial.println("sDOS_I2C::setup() A");
        this->connect();
        Serial.println("sDOS_I2C::setup() B");
        this->scan();
        Serial.println("sDOS_I2C::setup() C");
    };

    void loop() override {};

    bool isActive() override {
        return false;
    }

    static bool isConnected() {
        return sDOS_I2C::_isConnected;
    };

    void connect() {
        Serial.println("sDOS_I2C::connect() A");
#ifdef ESP32
        if (Wire.begin(I2C_SDA, I2C_SCL, I2C_CLOCK)) {
            Serial.println("sDOS_I2C::connect() B");
            this->_debugger->Debug("i2c", "I2C configured at %dkhz", (I2C_CLOCK / 1000));
            Serial.println("sDOS_I2C::connect() C");
            this->_eventsManager->trigger("i2c_ready");
            Serial.println("sDOS_I2C::connect() D");
            sDOS_I2C::_isConnected = true;
            Serial.println("sDOS_I2C::connect() E");
        } else {
            Serial.println("sDOS_I2C::connect() F");
            this->_eventsManager->trigger("i2c_fail");
            Serial.println("sDOS_I2C::connect() G");
        }
#endif
#ifdef ESP8266
        Wire.begin(I2C_SDA, I2C_SCL, I2C_CLOCK);
        this->_debugger->Debug("i2c", "I2C configured at %dkhz", (I2C_CLOCK/1000));
        this->_eventsManager->trigger("i2c_ready");
        sDOS_I2C::_isConnected = true;
#endif
        Serial.println("sDOS_I2C::connect() Z");
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
            Wire.beginTransmission(address);
            error = Wire.endTransmission();
            if (error == 0) {
                //Serial.printf("[i2c] > i2c device found at address %#04x!\n", address);
                _eventsManager->trigger("i2c_scan_found", address);
                nDevices++;
            }
        }

        _eventsManager->trigger("i2c_scan_end");
    };

    static bool i2cDeviceExists(byte address) {
        Wire.beginTransmission(address);
        return Wire.endTransmission() == 0;
    };

    String getName() {
        return _component;
    };

private:
    String _component = "i2c";
    Debugger * _debugger;
    EventsManager * _eventsManager;
    static bool _isConnected;
};

bool sDOS_I2C::_isConnected = false;
