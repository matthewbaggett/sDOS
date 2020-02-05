#ifndef ABSTRACT_RTC_H
#define ABSTRACT_RTC_H
#include "../kernel_inc.h"
#include "driver.h"
#include "RTClib.h"

class AbstractRTC  : public sDOS_Abstract_Driver
{
    public:
    virtual void setTime(DateTime & newTime) = 0;
    virtual void setAlarmInMinutes(int minutes);
    virtual void setAlarmInSeconds(int seconds);
    virtual DateTime getTime();
};
#endif