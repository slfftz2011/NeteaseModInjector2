#ifndef MODINJECTOR_NETWORKCHECKER_H
#define MODINJECTOR_NETWORKCHECKER_H


#pragma once
#include <string>
#include <vector>

class NetworkChecker {
public:
    enum class ConnectionStatus {
        CONNECTED,
        DISCONNECTED,
        LIMITED_ACCESS
    };

    NetworkChecker();

    // 基础网络连接检测
    static bool isConnected();

    // 详细网络状态检测
    static ConnectionStatus checkConnectionStatus();

    // 检测特定网站可达性
    static bool checkWebsiteReachable(const std::string& url);

    // 获取当前网络接口信息
    static std::vector<std::string> getNetworkInterfaces();

    // 获取最后错误信息
    [[nodiscard]] std::string getLastError() const;

private:
    std::string lastError;

    // 平台相关实现
    static bool platformPingTest(const std::string& target);
    static ConnectionStatus platformCheckStatus();
    static bool checkHttpReachable(const std::string& url);
    static std::string getHostFromUrl(const std::string& url);
};


#endif //MODINJECTOR_NETWORKCHECKER_H
