
#include "RegistryReader.h"
#include <stdexcept>

RegistryReader::RegistryReader() : lastError_("") {}

RegistryReader::~RegistryReader() {}

std::string RegistryReader::GetNeteaseDownloadPath() {
    const std::string keyPath = "Software\\Netease\\MCLauncher";
    const std::string valueName = "DownloadPath";
    
    return ReadRegistryString(HKEY_CURRENT_USER, keyPath, valueName);
}

std::string RegistryReader::ReadRegistryString(HKEY hKey, const std::string& subKey, const std::string& valueName) {
    HKEY hOpenedKey = nullptr;
    DWORD dwType = REG_SZ;
    char buffer[1024] = {0};
    DWORD bufferSize = sizeof(buffer);
    
    LONG result = RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hOpenedKey);
    if (result != ERROR_SUCCESS) {
        lastError_ = "Failed to open registry key: " + std::to_string(result);
        return "";
    }
    
    result = RegQueryValueExA(hOpenedKey, valueName.c_str(), nullptr, &dwType, (LPBYTE)buffer, &bufferSize);
    RegCloseKey(hOpenedKey);
    
    if (result != ERROR_SUCCESS) {
        lastError_ = "Failed to query registry value: " + std::to_string(result);
        return "";
    }
    
    return std::string(buffer);
}

std::string RegistryReader::GetLastError() const {
    return lastError_;
}
