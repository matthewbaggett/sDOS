#ifndef FILESYSTEM_CPP
#define FILESYSTEM_CPP
#include "../kernel_inc.h"
#include "FS.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"
#include <iostream>
#include <map>
#include <string>
#include "json_config_file.h"
#include "debugger.hpp"

class FileSystem
{
public:
    FileSystem(Debugger& debugger) : _debugger(&debugger) {};
    JsonConfigFile * loadJsonArray(JsonConfigFile * config, String fileName);

private:
    Debugger * _debugger;
    const String _component = "FS";
};

JsonConfigFile * FileSystem::loadJsonArray(JsonConfigFile * config, String fileName)
{
    if (!SPIFFS.begin()) {
        _debugger.Debug(_component, "loadJsonArray _spiffs.begin() for %s did not succeed\n", fileName.c_str());
    }
    File f = SPIFFS.open(fileName.c_str(), "r");
    if (!f) {
        _debugger.Debug(_component, "loadJsonArray _spiffs.open(\"%s\") failed\n", fileName.c_str());
    }

    // Read the file into buffer
    int documentSize = f.size() * 2.5;
    char temp[f.size()];
    f.readBytes(temp, f.size());
    f.close();

    DynamicJsonDocument doc(documentSize);
    DeserializationError error = deserializeJson(doc, temp);

    // Test if parsing succeeds.
    if (error) {
        _debugger.Debug(F("deserializeJson() failed: %s"), error.c_str());
        return config;
    }
    int rowId = 0;
    for (JsonObject row : doc.as<JsonArray>())
    {
        JsonConfigFile & _config = config[rowId];

        for (JsonObject::iterator it = row.begin(); it != row.end(); ++it)
        {
            const char *_key = it->key().c_str();
            const char *_value = it->value().as<char *>();
            _config.data.emplace(_key, _value);
        }

        rowId++;
    }

    return config;
}
#endif