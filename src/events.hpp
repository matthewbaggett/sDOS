#ifndef SDOS_EVENTS_HPP
#define SDOS_EVENTS_HPP
#include "kern_inc.h"
#include <RTClib.h> // Required for DateTime class

class EventsManager
{
public:
    EventsManager(Debugger &debugger);
    void trigger(String event, DateTime &payload);
    void trigger(String event, const __FlashStringHelper *payload);
    void trigger(String event, String payload);
    void trigger(String event, char *payload);
    void trigger(String event, long payload);
    void trigger(String event, double payload);
    void trigger(String event, uint32_t payload);
    void trigger(String event, int payload);
    void trigger(String event, byte payload);
    void trigger(String event, bool payload);
    void trigger(String event);
    void loop();

private:
    Debugger _debugger;
    String _component = "Events";
};

EventsManager::EventsManager(Debugger &debugger)
    : _debugger(debugger)
{
}

void EventsManager::loop()
{
}

void EventsManager::trigger(String event, DateTime &payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s : %s%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
};

void EventsManager::trigger(String event, const __FlashStringHelper *payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s : %s%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
};

void EventsManager::trigger(String event, String payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s : %s%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
};

void EventsManager::trigger(String event, char *payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s : %s%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
};

void EventsManager::trigger(String event, long payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s : %d%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
};

void EventsManager::trigger(String event, double payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s : %d%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
};

void EventsManager::trigger(String event, uint32_t payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s : %d%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
};

void EventsManager::trigger(String event, int payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s : %d%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
};

void EventsManager::trigger(String event, bool payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s : boolean(%s)%s", COL_GREY, event.c_str(), payload ? "TRUE" : "FALSE", COL_RESET);
#endif
};

void EventsManager::trigger(String event, byte payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s : %#04x%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
};

void EventsManager::trigger(String event)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s%s%s", COL_GREY, event.c_str(), COL_RESET);
#endif
};
#endif