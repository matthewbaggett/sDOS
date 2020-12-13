#pragma once
#include "driver/gpio.h"
// Hardware to enable:
// WiFi power saving:
#define WIFI_POWER_SAVING WIFI_PS_MAX_MODEM

// Enable i2c
#define ENABLE_I2C
#define I2C_SDA 5
#define I2C_SCL 4
#define I2C_CLOCK I2C_FREQ_DEFAULT

// Enable voltage monitoring
#define ENABLE_POWER

// Enable standard buttons
#define ENABLE_BUTTON

// Enable display
#define ENABLE_SSD1306
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 135

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

#define DEBUG_EVENTS
#define DEBUG_LOOP_RUNNING

