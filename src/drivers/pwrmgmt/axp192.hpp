#pragma once
#include "kern_inc.h"
#include "abstracts/driver.hpp"

#define sDOS_AXP192_DEFAULT_ADDRESS 0x34

typedef struct {
    bool EXTEN;
    bool BACKUP;
    int DCDC1;
    int DCDC2;
    int DCDC3;
    int LDO2;
    int LDO3;
    int GPIO0;
    int GPIO1;
    int GPIO2;
    int GPIO3;
    int GPIO4;
} sDOS_AXP192_InitDef;

class sDOS_AXP192 : public sDOS_Abstract_Driver {
private:
    uint8_t loopCount = 0;
    double previousBatteryVoltage = 0;
    double previousBatteryCurrent = 0;
    float previousBatteryDischargeCurrent = 0;
    float previousBatteryChargeCurrent = 0;
    float previousAcInVoltage = 0;
    float previousAcInCurrent = 0;
    float previousVbusVoltage = 0;
    float previousVbusCurrent = 0;
    float previousInternalTemperature = 0;
    float previousApsVoltage = 0;
public:
    sDOS_AXP192(Debugger * debugger, EventsManager * eventsManager, sDOS_I2C * i2c, uint8_t deviceAddress = sDOS_AXP192_DEFAULT_ADDRESS)
    : sDOS_Abstract_Driver(debugger, eventsManager), _i2c(i2c), _deviceAddress(deviceAddress) {
    }

    void setup(){
        this->_debugger->Debug(_component, "Startup");
    }

    bool isActive() override {
        this->loopCount++;
        if(this->loopCount < 150){
            return false;
        }
        this->loopCount = 0;
        return true;
    }

    void loop() {

        double newBatteryVoltage = this->getBatteryVoltage();
        if(this->previousBatteryVoltage != newBatteryVoltage){
            Serial.printf("Previous: %f\nCurrent: %f\n", this->previousBatteryVoltage, newBatteryVoltage);
            this->_debugger->Debug(_component, "Battery Voltage            : %7.2f => %7.2f delta %7.2f", this->previousBatteryVoltage, newBatteryVoltage, newBatteryVoltage - this->previousBatteryVoltage);
            this->previousBatteryVoltage = newBatteryVoltage;
            //this->_eventsManager->trigger("axp192_battery_voltage", newBatteryVoltage);
        }

        double newBatteryCurrent = this->getBatteryCurrent();
        if(this->previousBatteryCurrent != newBatteryCurrent){
            this->_debugger->Debug(_component, "Battery Current            : %7.2f => %7.2f delta %7.2f", this->previousBatteryCurrent, newBatteryCurrent, newBatteryCurrent - this->previousBatteryCurrent);
            this->previousBatteryCurrent = newBatteryCurrent;
            //this->_eventsManager->trigger("axp192_battery_current", newBatteryCurrent);
        }

        float newBatteryDischargeCurrent = this->getBatteryDischargeCurrent();
        if(this->previousBatteryDischargeCurrent != newBatteryDischargeCurrent){
            this->_debugger->Debug(_component, "Battery Current Draw       : %7.2f => %7.2f", this->previousBatteryDischargeCurrent, newBatteryDischargeCurrent);
            this->previousBatteryDischargeCurrent = newBatteryDischargeCurrent;
            //this->_eventsManager->trigger("axp192_battery_discharge_current", newBatteryDischargeCurrent);
        }

        float newBatteryChargeCurrent = this->getBatteryChargeCurrent();
        if(this->previousBatteryChargeCurrent != newBatteryChargeCurrent){
            this->_debugger->Debug(_component, "Battery Current (Charging) : %7.2f => %7.2f", this->previousBatteryChargeCurrent, newBatteryChargeCurrent);
            this->previousBatteryChargeCurrent = newBatteryChargeCurrent;
            //this->_eventsManager->trigger("axp192_battery_charge_current", newBatteryChargeCurrent);
        }

        float newAcInVoltage = this->getAcInVoltage();
        if(this->previousAcInVoltage != newAcInVoltage){
            //this->_debugger->Debug(_component, "Supplied Voltage           : %7.2f => %7.2f", this->previousAcInVoltage, newAcInVoltage);
            this->previousAcInVoltage = newAcInVoltage;
            //this->_eventsManager->trigger("axp192_supplied_voltage", newAcInVoltage);
        }

        float newAcInCurrent = this->getAcInCurrent();
        if(this->previousAcInCurrent != newAcInCurrent){
            //this->_debugger->Debug(_component, "Supplied Current           : %7.2f => %7.2f", this->previousAcInCurrent, newAcInCurrent);
            this->previousAcInCurrent = newAcInCurrent;
            //this->_eventsManager->trigger("axp192_supplied_current", newAcInCurrent);
        }

        float newVbusVoltage = this->getVbusVoltage();
        if(this->previousVbusVoltage != newVbusVoltage){
            //this->_debugger->Debug(_component, "VBUS Voltage               : %7.2f => %7.2f delta %7.2f", this->previousVbusVoltage, newVbusVoltage, newVbusVoltage - this->previousVbusVoltage);
            this->previousVbusVoltage = newVbusVoltage;
            //this->_eventsManager->trigger("axp192_vbus_voltage", newVbusVoltage);
        }

        float newVbusCurrent = this->getVbusCurrent();
        if(this->previousVbusCurrent != newVbusCurrent){
            //this->_debugger->Debug(_component, "VBUS Current               : %7.2f => %7.2f delta %7.2f", this->previousVbusCurrent, newVbusCurrent, newVbusCurrent - this->previousVbusCurrent);
            this->previousVbusCurrent = newVbusCurrent;
            //this->_eventsManager->trigger("axp192_vbus_current", newVbusCurrent);
        }

        float newInternalTemperature = this->getInternalTemperature();
        if(this->previousInternalTemperature != newInternalTemperature){
            //this->_debugger->Debug(_component, "Internal Temperature       : %7.2f => %7.2f", this->previousInternalTemperature, newInternalTemperature);
            this->previousInternalTemperature = newInternalTemperature;
            //this->_eventsManager->trigger("axp192_temperature", newInternalTemperature);
        }
    };

    String getName() {
        return _component;
    };

    void begin(sDOS_AXP192_InitDef initDef) {
        //ESP_LOGD("AXP192", "Begin");

        this->setEXTEN(initDef.EXTEN);
        this->setBACKUP(initDef.BACKUP);
        this->setDCDC1(initDef.DCDC1);
        this->setDCDC2(initDef.DCDC2);
        this->setDCDC3(initDef.DCDC3);

        this->setLDO2(initDef.LDO2);
        this->setLDO3(initDef.LDO3);

        this->setGPIO0(initDef.GPIO0);
        this->setGPIO1(initDef.GPIO1);
        this->setGPIO2(initDef.GPIO2);
        this->setGPIO3(initDef.GPIO3);
        this->setGPIO4(initDef.GPIO4);

        this->_i2c->write8bit(this->_deviceAddress, 0x28, 0xCC); // set LDO2 & LDO3 to 3.0v
        this->_i2c->write8bit(this->_deviceAddress, 0x82, 0b11111111);  // ADC All Enable
        this->_i2c->write8bit(this->_deviceAddress, 0x33, 0b11000000);  // Charge 4.2V, 100mA
        this->_i2c->write8bit(this->_deviceAddress, 0x84, 0b11110010);  // ADC 200Hz
        this->_i2c->write8bit(this->_deviceAddress, 0x12, 0b01111111);
        this->_i2c->write8bit(this->_deviceAddress, 0x36, 0b00001100);  // PEK 128ms, PW OFF 4S
        this->_i2c->write8bit(this->_deviceAddress, 0x91, 0xF0); // set RTC voltage to 3.3v
        this->_i2c->write8bit(this->_deviceAddress, 0x90,0x02); // GPIO0 to LDO
        this->_i2c->write8bit(this->_deviceAddress, 0x30, 0x80);  // Disable VBUS hold limit
        this->_i2c->write8bit(this->_deviceAddress, 0x39, 0xFC);  // Temp Protection
        this->_i2c->write8bit(this->_deviceAddress, 0x35, 0xA2);  // Enable RTC BAT charging
        this->_i2c->write8bit(this->_deviceAddress, 0x32, 0x46);  // Enable bat detection

        this->_i2c->write8bit(this->_deviceAddress, 0x31, 0b00000100);  // Power Off 3.0V

        this->_i2c->write8bit(this->_deviceAddress, 0xB8, 0b10000000); // Enable Colume Counter
    }

    void setDCDC1(int voltage) {
        uint8_t add = 0x12;
        uint8_t val;

        if (voltage < 700 || 3500 < voltage) {
            // Disable
            bitOff(add, (1 << 0));
            return;
        } else {
            // Enable
            bitOn(add, (1 << 0));
        }

        // Set
        add = 0x26;
        add = 0x26;
        val = ((voltage - 700) / 25) & 0x7f;
        this->_i2c->write8bit(this->_deviceAddress, add, val);
    }
    void setDCDC2(int voltage) {
        uint8_t add = 0x12;
        uint8_t val;

        if (voltage < 700 || 3500 < voltage) {
            // Disable
            bitOff(add, (1 << 4));
            return;
        } else {
            // Enable
            bitOn(add, (1 << 4));
        }

        // Set
        add = 0x23;
        val = ((voltage - 700) / 25) & 0x7f;
        this->_i2c->write8bit(this->_deviceAddress, add, val);
    }
    void setDCDC3(int voltage) {
        uint8_t add = 0x12;
        uint8_t val;

        if (voltage < 700 || 3500 < voltage) {
            // Disable
            bitOff(add, (1 << 1));
            return;
        } else {
            // Enable
            bitOn(add, (1 << 1));
        }

        // Set
        add = 0x27;
        val = ((voltage - 700) / 25) & 0x7f;
        this->_debugger->Debug(_component, "setLDO1(%d) => i2cWrite(%X, %X)", voltage, add, val);
        this->_i2c->write8bit(this->_deviceAddress, add, val);
    }

    void setLDO2(int voltage) {
        uint8_t add = 0x12;
        uint8_t val;

        if (voltage < 1800 || 3300 < voltage) {
            // Disable
            bitOff(add, (1 << 2));
            return;
        } else {
            // Enable
            bitOn(add, (1 << 2));
        }

        // Set
        add = 0x28;
        val = this->_i2c->read8bit(this->_deviceAddress, add) & 0x0f;
        val |= (((voltage - 1800) / 100) & 0x0f ) << 4;
        this->_debugger->Debug(_component, "setLDO2(%d) => i2cWrite(%X, %X)", voltage, add, val);
        this->_i2c->write8bit(this->_deviceAddress, add, val);
    }
    void setLDO3(int voltage) {
        uint8_t add = 0x12;
        uint8_t val;

        if (voltage < 1800 || 3300 < voltage) {
            // Disable
            bitOff(add, (1 << 3));
            return;
        } else {
            // Enable
            bitOn(add, (1 << 3));
        }

        // Set
        add = 0x28;
        val = this->_i2c->read8bit(this->_deviceAddress, add) & 0xf0;
        val |= (((voltage - 1800) / 100) & 0x0f );
        this->_debugger->Debug(_component, "setLDO3(%d) => i2cWrite(%X, %X)", voltage, add, val);
        this->_i2c->write8bit(this->_deviceAddress, add, val);
    }

    void setGPIO0(int voltage) {
        uint8_t add = 0x90;
        uint8_t val;

        if (voltage < 1800 || 3300 < voltage) {
            // Set Mode
            this->_i2c->write8bit(this->_deviceAddress, add, voltage);
        } else {
            // LDO
            this->_i2c->write8bit(this->_deviceAddress, add, 0b00000010);

            // Set voltage
            add = 0x91;
            val = (((voltage - 1800) / 100) & 0x0f ) << 4;
            this->_i2c->write8bit(this->_deviceAddress, add, val);
        }
    }
    void setGPIO1(int voltage) {};
    void setGPIO2(int voltage) {};
    void setGPIO3(int voltage) {};
    void setGPIO4(int voltage) {};

    void setEXTEN(bool enable) {
        uint8_t add = 0x12;
        if (enable) {
            // Enable
            this->bitOn(add, (1 << 6));
        } else {
            // Disable
            this->bitOff(add, (1 << 6));
        }
    }
    void setBACKUP(bool enable) {
        uint8_t add = 0x35;

        if (enable) {
            // Enable
            this->bitOn(add, (1 << 7));
        } else {
            // Disable
            this->bitOff(add, (1 << 7));
        }
    }

    double getBatteryVoltage() {
        double ADCLSB = 1.0 / 1000.0;
        uint16_t vbat = this->_i2c->read12bit(0x34, 0x78);
        Serial.print("VBAT from i2c: ");Serial.println(vbat);
        return vbat * ADCLSB;
    };

    double getBatteryCurrent() {
        double ADCLSB = 0.5;
        uint16_t currentIn = this->_i2c->read13bit(0x34, 0x7A);
        uint16_t currentOut = this->_i2c->read13bit(0x34, 0x7C);
        return (currentIn - currentOut) * ADCLSB;
    }

    uint16_t getBatteryDischargeCurrent() {
        uint16_t val = this->_i2c->read8bit(this->_deviceAddress, 0x7c) << 5;
        val |= this->_i2c->read8bit(this->_deviceAddress, 0x7d);
        return val;
    };
    uint16_t getBatteryChargeCurrent() {
        uint16_t val = this->_i2c->read8bit(this->_deviceAddress, 0x7a) << 5;
        val |= this->_i2c->read8bit(this->_deviceAddress, 0x7b);
        return val;
    };
    uint16_t getBatteryPower() {
        uint32_t val = (this->_i2c->read8bit(this->_deviceAddress, 0x70) << 16) | (this->_i2c->read8bit(this->_deviceAddress, 0x71) << 8) | this->_i2c->read8bit(this->_deviceAddress, 0x72);
        return  1.1 * 0.5 * val / 1000.0;
    };
    uint16_t getAcInVoltage() {
        uint16_t val = this->_i2c->read8bit(this->_deviceAddress, 0x56) << 4;
        val |= this->_i2c->read8bit(this->_deviceAddress, 0x57);
        return val;
    }
    uint16_t getAcInCurrent() {
        uint16_t val = this->_i2c->read8bit(this->_deviceAddress, 0x58) << 4;
        val |= this->_i2c->read8bit(this->_deviceAddress, 0x59);
        return val ;
    }

    uint16_t getVbusVoltage() {
        uint16_t val = this->_i2c->read8bit(this->_deviceAddress, 0x5a) << 4;
        val |= this->_i2c->read8bit(this->_deviceAddress, 0x5b);
        return val;
    }
    uint16_t getVbusCurrent() {
        uint16_t val = this->_i2c->read8bit(this->_deviceAddress, 0x5c) << 4;
        val |= this->_i2c->read8bit(this->_deviceAddress, 0x5d);
        return val;
    }
    uint16_t getInternalTemperature() {
        uint16_t val = this->_i2c->read8bit(this->_deviceAddress, 0x5e) << 4;
        val |= this->_i2c->read8bit(this->_deviceAddress, 0x5f);
        
        return -144.7 + val * 0.1;
    }

    uint16_t getApsVoltage() {
        uint16_t val = this->_i2c->read8bit(this->_deviceAddress, 0x7e) << 4;
        val |= this->_i2c->read8bit(this->_deviceAddress, 0x7f);
        return val;
    }

    void powerOff() {
        this->bitOn(0x32, (1 << 7));
    }
    uint8_t getPekPress() {
        uint8_t val = this->_i2c->read8bit(this->_deviceAddress, 0x46);
        this->_i2c->write8bit(this->_deviceAddress, 0x46, 0x03);
        return val;
    }

protected:
    String _component = "axp192";
    sDOS_I2C * _i2c;
    int _deviceAddress;

    void bitOn(uint8_t address, uint8_t bit) {
        uint8_t add = address;
        uint8_t val = this->_i2c->read8bit(this->_deviceAddress, add) | bit;
        this->_i2c->write8bit(this->_deviceAddress, add, val);
    }

    void bitOff(uint8_t address, uint8_t bit) {
        uint8_t add = address;
        uint8_t val = this->_i2c->read8bit(this->_deviceAddress, add) & ~bit;
        this->_i2c->write8bit(this->_deviceAddress, add, val);
    }

};
