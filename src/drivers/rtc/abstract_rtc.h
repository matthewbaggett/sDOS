#include "includes.h"

class AbstractRTC{
    public:
    virtual void setTime(DateTime & newTime) = 0;
    virtual void setAlarmInMinutes(int minutes);
    virtual DateTime getTime();
};
