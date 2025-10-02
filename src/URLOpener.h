
#ifndef MODINJECTOR_URLOPENER_H
#define MODINJECTOR_URLOPENER_H


#pragma once
#include <string>

class URLOpener {
public:
    static bool OpenURL(const std::string& url);
    static std::string GetLastError();

private:
    static std::string lastError;
};



#endif //MODINJECTOR_URLOPENER_H