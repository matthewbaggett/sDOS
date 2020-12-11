#include "kern_inc.h"
#include "abstracts/driver.hpp"

class sDOS_I2C : public sDOS_Abstract_Driver {
public:
    sDOS_I2C(Debugger &debugger, EventsManager &eventsManager) : _debugger(debugger), _events(eventsManager) {};

    void setup() override {
        connect();
        scan();
    };

    void loop() override {};

    bool isActive() override {
        return false;
    }

    static bool isConnected() {
        return sDOS_I2C::_isConnected;
    };

    void connect() {
#ifdef ESP32
        if (Wire.begin(I2C_SDA, I2C_SCL, I2C_CLOCK)) {
            _debugger.Debug("i2c", "I2C configured at %dkhz", (I2C_CLOCK / 1000));
            _events.trigger("i2c_ready");
            sDOS_I2C::_isConnected = true;
        } else {
            _events.trigger("i2c_fail");
        }
#endif
#ifdef ESP8266
        Wire.begin(I2C_SDA, I2C_SCL, I2C_CLOCK);
        _debugger.Debug("i2c", "I2C configured at %dkhz", (I2C_CLOCK/1000));
        _events.trigger("i2c_ready");
        sDOS_I2C::_isConnected = true;
#endif
    };

    TwoWire getWire() {
        if (!sDOS_I2C::isConnected()) {
            connect();
        }
        return Wire;
    };

    void scan() {
        _events.trigger("i2c_scan_begin");

        byte error, address;
        int nDevices;

        nDevices = 0;
        for (address = 1; address < 255; address++) {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();
            if (error == 0) {
                //Serial.printf("[i2c] > i2c device found at address %#04x!\n", address);
                _events.trigger("i2c_scan_found", address);
                nDevices++;
            }
        }

        _events.trigger("i2c_scan_end");
    };

    static bool i2cDeviceExists(byte address) {
        Wire.beginTransmission(address);
        return Wire.endTransmission() == 0;
    };

    String getName() override {
        return _component;
    };

private:
    String _component = "i2c";
    Debugger _debugger;
    EventsManager _events;
    static bool _isConnected;
};

bool sDOS_I2C::_isConnected = false;
