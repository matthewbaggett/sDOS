#include "includes.h"
#include "abstract_sdos_driver.h"

class AbstractRTC  : public sDOS_Abstract_Driver
{
    public:
    virtual void setTime(DateTime & newTime) = 0;
    virtual void setAlarmInMinutes(int minutes);
    virtual DateTime getTime();
};
