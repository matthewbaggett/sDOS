#include "kern_inc.h"
#include "driver.hpp"

class AbstractRTC  : public sDOS_Abstract_Driver
{
    public:
    virtual void setTime(DateTime & newTime) = 0;
    virtual void setAlarmInMinutes(int minutes);
    virtual void setAlarmInSeconds(int seconds);
    virtual DateTime getTime();
};
