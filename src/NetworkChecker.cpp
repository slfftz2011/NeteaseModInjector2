#include "NetworkChecker.h"
#include <cstdlib>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#include <iphlpapi.h>
#include <wininet.h>
// 仅在MSVC下使用#pragma comment，GCC忽略
#ifdef _MSC_VER
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wininet.lib")
#endif
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#endif

// ... 原有代码保持不变 ...

NetworkChecker::NetworkChecker() : lastError("") {}

bool NetworkChecker::isConnected() {
    return checkConnectionStatus() == ConnectionStatus::CONNECTED;
}

NetworkChecker::ConnectionStatus NetworkChecker::checkConnectionStatus() {
#ifdef _WIN32
    DWORD flags;
    if (InternetGetConnectedState(&flags, 0)) {
        return (flags & INTERNET_CONNECTION_LAN) ?
            ConnectionStatus::CONNECTED : ConnectionStatus::LIMITED_ACCESS;
    }
#else
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        lastError = "Socket creation failed";
        return ConnectionStatus::DISCONNECTED;
    }

    struct hostent* google = gethostbyname("www.google.com");
    if (google) {
        close(sock);
        return ConnectionStatus::CONNECTED;
    }
#endif
    return ConnectionStatus::DISCONNECTED;
}

bool NetworkChecker::checkWebsiteReachable(const std::string& url) {
    std::string targetUrl = url.empty() ? "http://www.google.com" : url;
    return checkHttpReachable(targetUrl);
}

std::vector<std::string> NetworkChecker::getNetworkInterfaces() {
    std::vector<std::string> interfaces;
#ifdef _WIN32
    auto adapterInfo = new IP_ADAPTER_INFO();
    ULONG bufLen = sizeof(IP_ADAPTER_INFO);

    if (GetAdaptersInfo(adapterInfo, &bufLen) == ERROR_BUFFER_OVERFLOW) {
        delete adapterInfo;
        adapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(new BYTE[bufLen]);
    }

    if (GetAdaptersInfo(adapterInfo, &bufLen) == NO_ERROR) {
        while (adapterInfo) {
            interfaces.emplace_back(adapterInfo->Description);
            adapterInfo = adapterInfo->Next;
        }
    }
    delete adapterInfo;
#endif
    return interfaces;
}

bool NetworkChecker::platformPingTest(const std::string& target) {
#ifdef _WIN32
    const std::string command = "ping -n 1 " + target + " > nul";
#else
    std::string command = "ping -c 1 " + target + " > /dev/null 2>&1";
#endif
    return system(command.c_str()) == 0;
}

bool NetworkChecker::checkHttpReachable(const std::string& url) {
#ifdef _WIN32
    HINTERNET hInternet = InternetOpenA("NetworkChecker", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        return false;
    }

    std::string host = getHostFromUrl(url);
    HINTERNET hConnect = InternetConnectA(hInternet, host.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return false;
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "HEAD", "/", NULL, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    BOOL result = HttpSendRequestA(hRequest, NULL, 0, NULL, 0);
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return result == TRUE;
#else
    std::string host = getHostFromUrl(url);
    struct hostent* server = gethostbyname(host.c_str());
    if (!server) {
        return false;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return false;
    }

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    std::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    int connectResult = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    close(sockfd);

    return connectResult == 0;
#endif
}

std::string NetworkChecker::getHostFromUrl(const std::string& url) {
    std::string host;
    size_t start = 0;
    if (url.compare(0, 7, "http://") == 0) {
        start = 7;
    } else if (url.compare(0, 8, "https://") == 0) {
        start = 8;
    }
    size_t end = url.find('/', start);
    if (end == std::string::npos) {
        host = url.substr(start);
    } else {
        host = url.substr(start, end - start);
    }
    return host;
}

std::string NetworkChecker::getLastError() const {
    return lastError;
}

NetworkChecker::ConnectionStatus NetworkChecker::platformCheckStatus() {
#ifdef _WIN32
    DWORD flags;
    if (InternetGetConnectedState(&flags, 0)) {
        if (flags & INTERNET_CONNECTION_LAN || flags & INTERNET_CONNECTION_MODEM) {
            return ConnectionStatus::CONNECTED;
        } else if (flags & INTERNET_CONNECTION_PROXY) {
            return ConnectionStatus::LIMITED_ACCESS;
        }
    }
    return ConnectionStatus::DISCONNECTED;
#else
    // 非Windows平台使用ping测试
    if (platformPingTest("8.8.8.8")) {
        return ConnectionStatus::CONNECTED;
    }
    lastError = "Unable to connect to the internet";
    return ConnectionStatus::DISCONNECTED;
#endif
}
