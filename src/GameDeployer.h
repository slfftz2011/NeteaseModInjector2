
#ifndef MODINJECTOR_GAMEDEPLOYER_H
#define MODINJECTOR_GAMEDEPLOYER_H


#pragma once
#include <string>
#include <vector>
#include <windows.h>

class GameDeployer {
public:
    // 构造函数：传入游戏目录和临时解压目录
    GameDeployer(const std::string& gameDir, const std::string& tempUnzipDir);

    // 复制解压后的文件到游戏目录
    bool deployFiles();

    // 监控游戏日志验证部署结果
    bool monitorGameLog(const std::string& logPath);

    // 终止Java进程
    bool killJavaProcesses();

    // 获取最后错误信息
    std::string getLastError() const;

private:
    std::string gameDirectory;       // 游戏安装目录
    std::string tempUnzipDirectory;  // 临时解压目录
    std::string lastError;           // 错误信息缓存

    // 复制单个目录到目标位置
    bool copyDirectory(const std::string& src, const std::string& dest);

    // 查找Java进程ID
    std::vector<DWORD> findJavaProcesses();
};

#endif //MODINJECTOR_GAMEDEPLOYER_H