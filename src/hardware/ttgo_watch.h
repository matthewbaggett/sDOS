#if TTGO_WATCH == HARDWARE
    // Enable i2c
    #define ENABLE_I2C
    #define I2C_SDA 21
    #define I2C_SCL 22
    #define I2C_CLOCK I2C_FREQ_DEFAULT

    // Enable TTP223 touch button driver
    #define ENABLE_TTP223
    #define PIN_INTERRUPT_TTP223 33
    #define PIN_POWER_TTP223 25

    // Enable PCF8563 RTC
    #define ENABLE_PCF8563
    #define PIN_INTERRUPT_PCF8563 34

    // Enable MPU9250 Accellerometer
    #define ENABLE_MPU9250
    #define PIN_INTERRUPT_MPU9250 39

    // Enable voltage monitoring
    #define ENABLE_POWER
    #define POWER_MONITOR_CHARGE_STATE 32
    #define POWER_MONITOR_VBATT 35
    #define POWER_MONITOR_VBUS 36
    #define POWER_MONITOR_DIVISOR 2

    // Enable activity light
    #define ENABLE_ACTIVITY_LIGHT 
    #define ACTIVITY_LIGHT_ONECOLOUR 4

#endif