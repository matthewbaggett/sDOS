#ifndef JSON_CONFIG_FILE_H
#define JSON_CONFIG_FILE_H
using namespace std;

typedef struct _JsonConfigFile {
    std::map<std::string, std::string> data;
} JsonConfigFile;

#endif