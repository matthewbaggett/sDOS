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
    void trigger(String event, boolean payload);
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
    _debugger.Debug(_component, "%s : %s", event.c_str(), payload.toStr());
#endif
};

void EventsManager::trigger(String event, const __FlashStringHelper *payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s : %s", event.c_str(), payload);
#endif
};

void EventsManager::trigger(String event, String payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s : %s", event.c_str(), payload.c_str());
#endif
};

void EventsManager::trigger(String event, char *payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s : %s", event.c_str(), payload);
#endif
};

void EventsManager::trigger(String event, long payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s : %d", event.c_str(), payload);
#endif
};

void EventsManager::trigger(String event, double payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s : %d", event.c_str(), payload);
#endif
};

void EventsManager::trigger(String event, uint32_t payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s : %d", event.c_str(), payload);
#endif
};

void EventsManager::trigger(String event, int payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s : %d", event.c_str(), payload);
#endif
};

void EventsManager::trigger(String event, boolean payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s : %d", event.c_str(), payload ? "true" : "false");
#endif
};

void EventsManager::trigger(String event, byte payload)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s : %#04x", event.c_str(), payload);
#endif
};

void EventsManager::trigger(String event)
{
#ifdef DEBUG_EVENTS
    _debugger.Debug(_component, "%s", event.c_str());
#endif
};
