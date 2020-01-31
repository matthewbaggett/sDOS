#include "includes.h"

#pragma message("Included SDOS_MPU9250")

class SDOS_MPU9250
{
public:
    SDOS_MPU9250(EventsManager &eventsManager);
    void setup();
    void loop();
    void enable();
    void disable();

private:
    static void interrupt();
    static bool hasInterruptOccured();
    static bool interruptTriggered;
    EventsManager _events;
    MPU9250_DMP _imu;
    void printIMUData();
    void checkFIFO();
    void checkInterrupt();
    void handleTap();
};

bool SDOS_MPU9250::interruptTriggered = false;

SDOS_MPU9250::SDOS_MPU9250(EventsManager &eventsManager) : _events(eventsManager)
{
}

void SDOS_MPU9250::setup()
{
    _events.trigger("MPU9250_enable");

    // Initialise IMU
    if (_imu.begin() != INV_SUCCESS)
    {
        _events.trigger("MPU9250_fail");
        delay(5000);
        return;
    }

    // Setup DMP features.
    _imu.dmpBegin(DMP_FEATURE_TAP, 10);

    //_imu.setSampleRate(100);

    //_imu.configureFifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);

    unsigned short xThresh = 100;
    unsigned short yThresh = 100;
    unsigned short zThresh = 100;
    unsigned char taps = 1;         // Set minimum taps to 1
    unsigned short tapTime = 100;   // Set tap time to 100ms
    unsigned short tapMulti = 1000; // Set multi-tap time to 1s
    _imu.dmpSetTap(xThresh, yThresh, zThresh, taps, tapTime, tapMulti);

    // Setup DMP interrupt
    _imu.enableInterrupt();
    _imu.setIntLevel(INT_ACTIVE_LOW);
    //_imu.setIntLatched(false);
    pinMode(PIN_INTERRUPT_MPU9250, INPUT);
    attachInterrupt(PIN_INTERRUPT_MPU9250, SDOS_MPU9250::interrupt, FALLING);

    // All done
    _events.trigger("MPU9250_ready");
};

void SDOS_MPU9250::interrupt()
{
    interruptTriggered = true;
};

bool SDOS_MPU9250::hasInterruptOccured()
{
    if (interruptTriggered)
    {
        interruptTriggered = false;
        return true;
    }
    return false;
}

void SDOS_MPU9250::loop()
{
    //checkInterrupt();
    checkFIFO();
};

void SDOS_MPU9250::checkInterrupt()
{
    if (SDOS_MPU9250::hasInterruptOccured())
    {
        _events.trigger("MPU9250_interrupt");
        _imu.update(UPDATE_ACCEL | UPDATE_GYRO | UPDATE_COMPASS);
        printIMUData();
    }
}

void SDOS_MPU9250::checkFIFO()
{
    //Serial.printf("Fifo length: %d\n", _imu.fifoAvailable());
    if (_imu.fifoAvailable() >= 0)
    {
        //Serial.printf("Fifo length: %d\n", _imu.fifoAvailable());

        while (_imu.fifoAvailable() > 0)
        {
            yield();
            // DMP FIFO must be updated in order to update tap data
            if (_imu.dmpUpdateFifo() != INV_SUCCESS)
            {
                _events.trigger("mpu9250_fifo", "failure_to_read");
            }

            // Check for new tap data by polling tapAvailable
            if (_imu.tapAvailable())
            {
                handleTap();
            }
        }
    }
}

void SDOS_MPU9250::handleTap()
{
    _events.trigger("mpu9250_tap", "available");
    // If a new tap happened, get the direction and count
    // by reading getTapDir and getTapCount
    unsigned char tapDir = _imu.getTapDir();
    unsigned char tapCnt = _imu.getTapCount();
    switch (tapDir)
    {
    case TAP_X_UP:
        _events.trigger("mpu9250_tap", "X+");
        break;
    case TAP_X_DOWN:
        _events.trigger("mpu9250_tap", "X-");
        break;
    case TAP_Y_UP:
        _events.trigger("mpu9250_tap", "Y+");
        break;
    case TAP_Y_DOWN:
        _events.trigger("mpu9250_tap", "Y-");
        break;
    case TAP_Z_UP:
        _events.trigger("mpu9250_tap", "Z+");
        break;
    case TAP_Z_DOWN:
        _events.trigger("mpu9250_tap", "Z-");
        break;
    }
}

void SDOS_MPU9250::enable()
{
    _events.trigger("MPU9250_enable");
    //@todo power management code
}

void SDOS_MPU9250::disable()
{
    _events.trigger("MPU9250_disable");
    //@todo power management code
}

void SDOS_MPU9250::printIMUData(void)
{
    // After calling update() the ax, ay, az, gx, gy, gz, mx,
    // my, mz, time, and/or temerature class variables are all
    // updated. Access them by placing the object. in front:

    // Use the calcAccel, calcGyro, and calcMag functions to
    // convert the raw sensor readings (signed 16-bit values)
    // to their respective units.
    float accelX = _imu.calcAccel(_imu.ax);
    float accelY = _imu.calcAccel(_imu.ay);
    float accelZ = _imu.calcAccel(_imu.az);
    float gyroX = _imu.calcGyro(_imu.gx);
    float gyroY = _imu.calcGyro(_imu.gy);
    float gyroZ = _imu.calcGyro(_imu.gz);
    float magX = _imu.calcMag(_imu.mx);
    float magY = _imu.calcMag(_imu.my);
    float magZ = _imu.calcMag(_imu.mz);

    Serial.println("Accel: " + String(accelX) + ", " + String(accelY) + ", " + String(accelZ) + " g");
    Serial.println("Gyro: " + String(gyroX) + ", " + String(gyroY) + ", " + String(gyroZ) + " dps");
    //Serial.println("Mag: " + String(magX) + ", " + String(magY) + ", " + String(magZ) + " uT");
    //Serial.println("Time: " + String(_imu.time) + " ms");
    Serial.println();
}
