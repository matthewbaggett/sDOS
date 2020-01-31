#include "includes.h"
class FileSystem{
    public:
        FileSystem(Debugger & debugger){
            _debugger = debugger;
        }
        JsonArray loadJsonArray(String fileName);
    private:
        Debugger _debugger;
        String _component = "FS";
};

JsonArray FileSystem::loadJsonArray(String fileName) {
    
            DynamicJsonBuffer jsonBuff;
            
            if(!SPIFFS.begin()){
                _debugger.Debug(_component, "loadJsonArray _spiffs.begin() for %s did not succeed\n", fileName.c_str());
            }
            File f = SPIFFS.open(fileName.c_str(), "r");
            if (!f) {
                _debugger.Debug(_component, "loadJsonArray _spiffs.open(\"%s\") failed\n", fileName.c_str());
            }
            char temp[f.size()];
            f.readBytes(temp, f.size());
            JsonArray jsonData = jsonBuff.parseArray(temp);
            f.close();
            return jsonData;
        
}
