#include "includes.h"

class EventsManager
{
public:
    EventsManager(Debugger &debugger);
    void trigger(String event, DateTime & payload);
    void trigger(String event, const __FlashStringHelper * payload);
    void trigger(String event, String payload);
    void trigger(String event, char * payload);
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

void EventsManager::trigger(String event, DateTime & payload)
{
    _debugger.Debug(_component, "%s : %s", event.c_str(), payload.toStr());
};

void EventsManager::trigger(String event, const __FlashStringHelper * payload)
{
    _debugger.Debug(_component, "%s : %s", event.c_str(), payload);
};

void EventsManager::trigger(String event, String payload)
{
    _debugger.Debug(_component, "%s : %s", event.c_str(), payload.c_str());
};

void EventsManager::trigger(String event, char *payload)
{
    _debugger.Debug(_component, "%s : %s", event.c_str(), payload);
};

void EventsManager::trigger(String event, long payload)
{
    _debugger.Debug(_component, "%s : %d", event.c_str(), payload);
};

void EventsManager::trigger(String event, double payload)
{
    _debugger.Debug(_component, "%s : %d", event.c_str(), payload);
};

void EventsManager::trigger(String event, uint32_t payload)
{
    _debugger.Debug(_component, "%s : %d", event.c_str(), payload);
};

void EventsManager::trigger(String event, int payload)
{
    _debugger.Debug(_component, "%s : %d", event.c_str(), payload);
};

void EventsManager::trigger(String event, boolean payload)
{
    _debugger.Debug(_component, "%s : %d", event.c_str(), payload ? "true" : "false");
};

void EventsManager::trigger(String event, byte payload)
{
    _debugger.Debug(_component, "%s : %#04x", event.c_str(), payload);
};

void EventsManager::trigger(String event)
{
    _debugger.Debug(_component, "%s", event.c_str());
};
