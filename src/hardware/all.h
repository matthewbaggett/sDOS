
#define XSTR(x) STR(x)
#define STR(x) #x

#define ESP8266_DEVBOARD 53
#define TTGO_DEVBOARD 54
#define TTGO_WATCH 55

//#pragma message "The value of HARDWARE: " XSTR(HARDWARE)

#if TTGO_DEVBOARD == HARDWARE
#include "ttgo_devboard/ttgo_devboard.h"
#endif

#if TTGO_WATCH == HARDWARE
#include "ttgo_watch/ttgo_watch.h"
#endif

#if ESP8266_DEVBOARD == HARDWARE
#include "esp8266_devboard/esp8266_devboard.h"
#endif
