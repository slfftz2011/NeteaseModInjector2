
#include "URLOpener.h"
#include <cstdlib>
#include <regex>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#elif __APPLE__
#include <CoreFoundation/CFBundle.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

std::string URLOpener::lastError;

std::string URLOpener::GetLastError() {
    return lastError;
}

bool URLOpener::OpenURL(const std::string& url) {
    // 简单的URL格式验证
    if (const std::regex urlRegex(R"(^(https?|ftp)://[^\s/$.?#].[^\s]*$)", std::regex::icase); !std::regex_match(url, urlRegex)) {
        lastError = "无效的URL格式";
        return false;
    }

#ifdef _WIN32
    if (HINSTANCE result = ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL); reinterpret_cast<intptr_t>(result) <= 32) {
        lastError = "Windows API执行失败";
        return false;
    }
#elif __APPLE__
    CFURLRef cfUrl = CFURLCreateWithBytes(
        NULL,
        (UInt8*)url.c_str(),
        url.length(),
        kCFStringEncodingUTF8,
        NULL
    );

    if (!cfUrl) {
        lastError = "创建CFURL失败";
        return false;
    }

    OSStatus status = LSOpenCFURLRef(cfUrl, NULL);
    CFRelease(cfUrl);

    if (status != noErr) {
        lastError = "MacOS打开URL失败";
        return false;
    }
#else
    std::string command = "xdg-open " + url;
    if (system(command.c_str()) != 0) {
        lastError = "Linux打开URL失败";
        return false;
    }
#endif
    return true;
}
