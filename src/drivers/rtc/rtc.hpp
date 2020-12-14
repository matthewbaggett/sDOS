#pragma once
#include "abstracts/rtc.hpp"

class sDOS_RTC : public AbstractRTC {
public:
    sDOS_RTC(Debugger *debugger, EventsManager *eventsManager) : AbstractRTC(debugger, eventsManager) {}

};

