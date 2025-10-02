
#ifndef MODINJECTOR_REGISTRYREADER_H
#define MODINJECTOR_REGISTRYREADER_H


#pragma once
#include <string>
#include <windows.h>

class RegistryReader {
public:
    RegistryReader();
    ~RegistryReader();

    std::string GetNeteaseDownloadPath();
    std::string GetLastError() const;

private:
    std::string lastError_;

    std::string ReadRegistryString(HKEY hKey, const std::string& subKey, const std::string& valueName);
};


#endif //MODINJECTOR_REGISTRYREADER_H