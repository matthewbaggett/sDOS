#pragma once
#include "../kern_inc.h"
#include <ArduinoJson.h>
#include <FS.h>

#ifdef ESP32
#include <SPIFFS.h>
#endif

#include <iostream>
#include <map>
#include <string>
#include "../abstracts/driver.hpp"

using namespace std;

struct JsonConfigFile {
    std::map<std::string, std::string> data;
};

class FileSystem : public sDOS_Abstract_Driver {
private:
    const String _component = "SPIFFS";
    bool firstLoop = true;

public:
    explicit FileSystem(Debugger * debugger, EventsManager * eventsManager) : sDOS_Abstract_Driver(debugger, eventsManager) {
        //debugger->Debug(_component, "Construct");
    };

    void setup() override {
        this->_debugger->Debug(_component, "Starting %sSPIFFS%s", COL_RED, COL_RESET);
        SPIFFS.begin();
        this->_debugger->Debug(_component, "Starting %sSPIFFS%s complete!", COL_RED, COL_RESET);
    }

    bool isActive() override {
        return false;
    }

    String getName() override {
        return _component;
    };

    void loop() override {}

    JsonConfigFile *loadJsonArray(JsonConfigFile *config, const String& fileName) {
        this->_debugger->Debug(_component, "Reading %s%s%s from %sSPIFFS%s", COL_GREEN, fileName.c_str(), COL_RESET, COL_RED,
                        COL_RESET);
        this->_debugger->Debug(_component, "%sSPIFFS%s init OK", COL_RED, COL_RESET);
        if (!SPIFFS.exists(fileName.c_str())) {
            this->_debugger->Debug(_component, "File %s%s%s DOES NOT EXIST in %sSPIFFS%s", COL_GREEN, fileName.c_str(), COL_RESET,
                            COL_RED, COL_RESET);
        }
        this->_debugger->Debug(_component, "File %s%s%s exists in %sSPIFFS%s", COL_GREEN, fileName.c_str(), COL_RESET, COL_RED,
                        COL_RESET);
        File f = SPIFFS.open(fileName.c_str(), "r");
        if (!f) {
            this->_debugger->Debug(_component, "loadJsonArray _spiffs.open(\"%s\") failed", fileName.c_str());
        }

        // Read the file into buffer
        int documentSize = f.size() * 2.5;
        char temp[f.size()];
        f.readBytes(temp, f.size());
        f.close();
        yield();

        DynamicJsonDocument doc(documentSize);
        DeserializationError error = deserializeJson(doc, temp);
        // Test if parsing succeeds.
        if (error) {
            this->_debugger->Debug(F("deserializeJson() failed: %s"), error.c_str());
            return config;
        }
        int rowId = 0;
        for (JsonObject row : doc.as<JsonArray>()) {
            JsonConfigFile &_config = config[rowId];

            for (JsonObject::iterator it = row.begin(); it != row.end(); ++it) {
                const char *_key = it->key().c_str();
                const char *_value = it->value().as<char *>();
                _config.data.emplace(_key, _value);
                yield();
            }

            rowId++;
        }

        return config;
    };


};
