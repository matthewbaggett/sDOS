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
#define SPI_MISO GPIO_NUM_38 // Hack, the TFT doesn't need one.
#define SPI_SS GPIO_NUM_5

// Enable voltage monitoring
#define ENABLE_POWER
#define POWER_MONITOR_VBATT GPIO_NUM_34
#define POWER_MONITOR_DIVISOR 2

// Enable standard buttons
#define ENABLE_BUTTON
#define BUTTON_0 GPIO_NUM_0
#define BUTTON_1 GPIO_NUM_35

// Enable display
#define ENABLE_ST7789
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 135
#define DISPLAY_OFFSET_COLS 40
#define DISPLAY_OFFSET_ROWS 53
#define ST77XX_DC GPIO_NUM_16
#define ST77XX_RST GPIO_NUM_23
#define ST77XX_CS SPI_SS
#define ST77XX_BL GPIO_NUM_4

// Enable fake RTC device
#define ENABLE_FAKE_RTC

// Services to enable:
// NTP service
#define ENABLE_SERVICE_NTP

// Sleeptune automatic sleep optimmiser
#define ENABLE_SERVICE_SLEEPTUNE
#define SLEEPTUNE_LOOPS_PER_SECOND 10

// Enable CPU frequency scaling
#define ENABLE_CPU_SCALER
#define CPU_FREQ_ONDEMAND 240

#define DEBUG_EVENTS
//#define DEBUG_LOOP_RUNNING

