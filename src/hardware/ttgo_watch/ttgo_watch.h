#include "driver/gpio.h"

// Hardware to enable:
// WiFi power saving:
#define WIFI_POWER_SAVING WIFI_PS_MAX_MODEM

// Enable i2c
#define ENABLE_I2C
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22
#define I2C_CLOCK I2C_FREQ_DEFAULT


// Enable SPI
#define ENABLE_SPI
#define SPI_SCLK GPIO_NUM_18
#define SPI_MOSI GPIO_NUM_19
#define SPI_SS GPIO_NUM_5

// Enable TTP223 touch button driver
#define ENABLE_TTP223
#define PIN_INTERRUPT_TTP223 GPIO_NUM_33
#define PIN_POWER_TTP223 GPIO_NUM_25

// Enable PCF8563 RTC
#define ENABLE_PCF8563
#define PIN_INTERRUPT_PCF8563 GPIO_NUM_34

// Enable MPU9250 Accellerometer
#define ENABLE_MPU9250
#define PIN_INTERRUPT_MPU9250 GPIO_NUM_39

// Enable voltage monitoring
#define ENABLE_POWER
#define POWER_MONITOR_CHARGE_STATE GPIO_NUM_32
#define POWER_MONITOR_VBATT GPIO_NUM_35
#define POWER_MONITOR_VBUS GPIO_NUM_36
#define POWER_MONITOR_DIVISOR 2

// Enable activity light
#define ENABLE_MONOCOLOUR_LED GPIO_NUM_4

// Enable display
#define ENABLE_ST7735
#define DISPLAY_WIDTH 160
#define DISPLAY_HEIGHT 80
#define DISPLAY_OFFSET_COLS 40
#define DISPLAY_OFFSET_ROWS 0
#define ST77XX_CS GPIO_NUM_5
#define ST77XX_DC GPIO_NUM_23
#define ST77XX_RST GPIO_NUM_26
#define ST77XX_BL GPIO_NUM_27

// Services to enable:
// NTP service
#define ENABLE_SERVICE_NTP

// Sleeptune automatic sleep optimmiser
#define ENABLE_SERVICE_SLEEPTUNE
#define SLEEPTUNE_LOOPS_PER_SECOND 10

// Enable CPU frequency scaling
#define ENABLE_CPU_SCALER

#define DEBUG_EVENTS
//#define DEBUG_LOOP_RUNNING
