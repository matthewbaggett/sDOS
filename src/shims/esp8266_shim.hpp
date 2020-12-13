#pragma once
#ifdef ESP8266
uint8_t getCpuFrequencyMhz() {
    return ESP.getCpuFreqMHz();
}
#endif