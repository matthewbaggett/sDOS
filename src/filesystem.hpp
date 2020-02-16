#include "kern_inc.h"
#include <ArduinoJson.h>
#include <FS.h>

#ifdef ESP32

#include <SPIFFS.h>

#endif

#include <iostream>
#include <map>
#include <string>

using namespace std;

struct JsonConfigFile {
    std::map<std::string, std::string> data;
};

class FileSystem {
    const int MAX_PROBABLE_CONFIGURED_WIFIS = 20;

public:
    FileSystem(Debugger &debugger) {
        _debugger = debugger;
    }

    JsonConfigFile *loadJsonArray(JsonConfigFile *config, String fileName);

private:
    Debugger _debugger;
    const String _component = "FS";
};

JsonConfigFile *FileSystem::loadJsonArray(JsonConfigFile *config, String fileName) {
    _debugger.Debug(_component, "Reading %s%s%s from %sSPIFFS%s", COL_GREEN, fileName.c_str(), COL_RESET, COL_RED,
                    COL_RESET);
    delay(1000);
    if (!SPIFFS.begin(true)) {
        _debugger.Debug(_component, "Reading %s%s%s from %sSPIFFS%s has %sFAILED%s", COL_GREEN, fileName.c_str(),
                        COL_RESET, COL_RED, COL_RESET, COL_RED, COL_RESET);
    }
    _debugger.Debug(_component, "%sSPIFFS%s init OK", COL_RED, COL_RESET);
    if (!SPIFFS.exists(fileName.c_str())) {
        _debugger.Debug(_component, "File %s%s%s DOES NOT EXIST in %sSPIFFS%s", COL_GREEN, fileName.c_str(), COL_RESET,
                        COL_RED, COL_RESET);
    }
    _debugger.Debug(_component, "File %s%s%s exists in %sSPIFFS%s", COL_GREEN, fileName.c_str(), COL_RESET, COL_RED,
                    COL_RESET);
    File f = SPIFFS.open(fileName.c_str(), "r");
    if (!f) {
        _debugger.Debug(_component, "loadJsonArray _spiffs.open(\"%s\") failed", fileName.c_str());
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
        _debugger.Debug(F("deserializeJson() failed: %s"), error.c_str());
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
}
