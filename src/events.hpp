#ifndef SDOS_EVENTS_HPP
#define SDOS_EVENTS_HPP

#include "kern_inc.h"
#include "debugger.hpp"
#include <RTClib.h> // Required for DateTime class
#include <functional>
#include <iostream>
#include <vector>
#include <abstracts/service.hpp>

using namespace std;

struct EventHandler {
    String name;
    std::function<void(String payload)> callback;
};

using eventHandlersList = std::list<EventHandler>;

class EventsManager : public sDOS_Abstract_Service {
public:
    explicit EventsManager(Debugger &debugger)
        : _debugger(debugger) {};

    void setup() override {};
    void loop() override {};

    static void on(String event, std::function<void(String payload)> newHandler) {
        EventHandler eventHandler;
        eventHandler.name = event;
        eventHandler.callback = newHandler;
        EventsManager::_handlers.push_back(eventHandler);
    };

    void trigger(String event, DateTime &payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : %s%s", COL_GREY, event.c_str(), payload.toStr(), COL_RESET);
#endif
        findCallbacks(event, payload.toStr());
    };

    void trigger(String event, const __FlashStringHelper *payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : %s%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
        findCallbacks(event, payload);
    };

    void trigger(String event, String payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : %s%s", COL_GREY, event.c_str(), payload.c_str(), COL_RESET);
#endif
        findCallbacks(event, payload);
    };

    void trigger(String event, char *payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : %s%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
        findCallbacks(event, payload);
    };

    void trigger(String event, long payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : %d%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
        findCallbacks(event, String(static_cast<char>(payload)));
    };

    void trigger(String event, double payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : %d%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
        findCallbacks(event, String(static_cast<char>(payload)));
    };

    void trigger(String event, uint32_t payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : %d%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
        findCallbacks(event, String(static_cast<char>(payload)));
    };

    void trigger(String event, uint64_t payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : %d%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
        findCallbacks(event, String(static_cast<char>(payload)));
    };

    void trigger(String event, int payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : %d%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
        findCallbacks(event, String(static_cast<char>(payload)));
    };

    void trigger(String event, bool payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : boolean(%s)%s", COL_GREY, event.c_str(), payload ? "TRUE" : "FALSE", COL_RESET);
#endif
        findCallbacks(event, String(static_cast<char>(payload)));
    };

    void trigger(String event, byte payload) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s : %#04x%s", COL_GREY, event.c_str(), payload, COL_RESET);
#endif
        findCallbacks(event, String(static_cast<char>(payload)));
    };

    void trigger(String event) {
#ifdef DEBUG_EVENTS
        _debugger.Debug(_component, "%s%s%s", COL_GREY, event.c_str(), COL_RESET);
#endif
        findCallbacks(event, "");
    };
private:
    Debugger _debugger;
    String _component = "Events";
    static eventHandlersList _handlers;

    void findCallbacks(String eventName, String payload = "") {
        for (auto const &handler : _handlers) {
            int diff = strcmp(handler.name.c_str(), eventName.c_str());
            if (diff == 0) {
                handler.callback(payload);
                //_debugger.Debug(_component, "%sEvent detected: %s. Handler matches %s%s.", COL_GREEN, eventName.c_str(),
                //                handler.name.c_str(), COL_RESET);
            }
        }
    };
};

eventHandlersList EventsManager::_handlers;

#endif