#include "includes.h"

class SDOS_I2C : public sDOS_Abstract_Driver
{
public:
    SDOS_I2C(Debugger &debugger, EventsManager &eventsManager);
    void setup();
    void loop();
    //static TwoWire wire;
    static bool isConnected();
    void connect();
    TwoWire getWire();
    void scan();
    bool i2cDeviceExists(byte address);
    
private:
    Debugger _debugger;
    EventsManager _events;
    static bool _isConnected;
};

//TwoWire SDOS_I2C::wire = Wire;
bool SDOS_I2C::_isConnected = false;

SDOS_I2C::SDOS_I2C(Debugger &debugger, EventsManager &eventsManager) :  _debugger(debugger), _events(eventsManager)
{
};

void SDOS_I2C::setup(){
    connect();
    scan();
};

bool SDOS_I2C::isConnected(){
    return SDOS_I2C::_isConnected;
}

void SDOS_I2C::connect(){
    if(Wire.begin(I2C_SDA, I2C_SCL, I2C_CLOCK)){
        _debugger.Debug("i2c", "I2C configured at %dkhz", (I2C_CLOCK/1000));
        _events.trigger("i2c_ready");
        SDOS_I2C::_isConnected = true;
    }else{
        _events.trigger("i2c_fail");
    }   
}

TwoWire SDOS_I2C::getWire(){
    if(!SDOS_I2C::isConnected()){
        connect();
    }
    return Wire;
}

void SDOS_I2C::scan(){
    _events.trigger("i2c_scan_begin");

    byte error, address;
    int nDevices;
    
    //Serial.println("[i2c] Scanning i2c bus...");
    nDevices = 0;
    for(address = 1; address < 255; address++ ) 
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0){
            //Serial.printf("[i2c] > i2c device found at address %#04x!\n", address);
            _events.trigger("i2c_scan_found", address);
            nDevices++;
        } else if (error==4) {
            //Serial.printf("[i2c] > Unknown error at address %#04x\n", address);
        }
    }
    
    if (nDevices == 0){
        //Serial.println("[i2c] > No I2C devices found");
    }else{
        //Serial.println("[i2c] > Done!");
    }
    _events.trigger("i2c_scan_end");
}

bool SDOS_I2C::i2cDeviceExists(byte address){
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

void SDOS_I2C::loop()
{
};

