# sDOS
Smol Device Operating System

Features:
  * Accellerometer Support:
    * MPU9250
  * RTC Support:
    * PCF8563
    * Fake RTC for when your real RTC went out for a packet of smokes
  * LCD Support:
    * ST7735 16bit "565" colour LCD
    * ST7789 16bit "565" colour LCD
    * SSD1311 mono oLED (future)
  * Genericised framebuffer interface
    * Draw to a generic framebuffer interface, abstracting away the display processing
    * Use class primatives for things like coordinates and colour, easing coordinate maths and LCD compatability
  * Integrated Services
    * OTA Update
    * NTP Daemon to update RTC time to internet time.
    * CPU Scaler to clock-down the microprocessor, or boost speed on-demand for intensive tasks
    * Dynamic sleep interval control - Dynamically sleeps (when radios not active) as much as possible, while retaining a number of loop ticks per second
  * Bluetooth BLE Support
    * BLE UART debug console