#include "GameDeployer.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <tlhelp32.h>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

GameDeployer::GameDeployer(const std::string& gameDir, const std::string& tempUnzipDir)
    : gameDirectory(gameDir), tempUnzipDirectory(tempUnzipDir) {}

bool GameDeployer::deployFiles() {
    try {
        // 复制mods目录
        std::string srcMods = tempUnzipDirectory + "\\mods";
        std::string destMods = gameDirectory + "\\mods";
        if (!copyDirectory(srcMods, destMods)) {
            lastError = "复制mods目录失败";
            return false;
        }

        // 复制config目录
        std::string srcConfig = tempUnzipDirectory + "\\config";
        std::string destConfig = gameDirectory + "\\config";
        if (!copyDirectory(srcConfig, destConfig)) {
            lastError = "复制config目录失败";
            return false;
        }

        // 复制resourcepacks目录
        std::string srcResource = tempUnzipDirectory + "\\resourcepacks";
        std::string destResource = gameDirectory + "\\resourcepacks";
        if (!copyDirectory(srcResource, destResource)) {
            lastError = "复制resourcepacks目录失败";
            return false;
        }

        return true;
    } catch (const fs::filesystem_error& e) {
        lastError = "文件操作错误: " + std::string(e.what());
        return false;
    }
}

bool GameDeployer::monitorGameLog(const std::string& logPath) {
    // 等待日志文件创建（最多等待60秒）
    int waitCount = 0;
    while (!fs::exists(logPath) && waitCount < 60) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        waitCount++;
    }

    if (!fs::exists(logPath)) {
        lastError = "未找到游戏日志文件: " + logPath;
        return false;
    }

    // 监控日志中"成功加载"的标志（根据实际日志内容调整）
    std::ifstream logFile(logPath, std::ios::ate);
    if (!logFile.is_open()) {
        lastError = "无法打开日志文件";
        return false;
    }

    auto logSize = logFile.tellg();
    std::string line;
    bool deployed = false;

    // 最多监控300秒
    for (int i = 0; i < 300; i++) {
        logFile.seekg(0, std::ios::end);
        if (logFile.tellg() > logSize) {
            logFile.seekg(logSize);
            while (std::getline(logFile, line)) {
                if (line.find("Successfully loaded all mods") != std::string::npos) {
                    // 检测到加载完成标志，执行部署
                    deployed = deployFiles();
                    break;
                }
            }
            logSize = logFile.tellg();
        }

        if (deployed) break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (!deployed) {
        lastError = "游戏启动超时或未检测到加载完成标志";
        return false;
    }

    return true;
}

bool GameDeployer::killJavaProcesses() {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        lastError = "无法获取进程快照";
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        lastError = "无法获取进程信息";
        return false;
    }

    bool killed = false;
    do {
        if (std::string(pe32.szExeFile) == "javaw.exe" || std::string(pe32.szExeFile) == "java.exe") {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess != NULL) {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
                killed = true;
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return killed;
}

std::string GameDeployer::getLastError() const {
    return lastError;
}

bool GameDeployer::copyDirectory(const std::string& src, const std::string& dest) {
    if (!fs::exists(src)) return true;  // 源目录不存在则跳过

    if (!fs::exists(dest)) {
        fs::create_directories(dest);
    }

    for (const auto& entry : fs::directory_iterator(src)) {
        const auto& srcPath = entry.path();
        const auto& destPath = fs::path(dest) / srcPath.filename();

        if (entry.is_directory()) {
            if (!copyDirectory(srcPath.string(), destPath.string())) {
                return false;
            }
        } else {
            fs::copy_file(srcPath, destPath, fs::copy_options::overwrite_existing);
        }
    }

    return true;
}

std::vector<DWORD> GameDeployer::findJavaProcesses() {
    std::vector<DWORD> pids;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) return pids;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return pids;
    }

    do {
        if (std::string(pe32.szExeFile) == "javaw.exe" || std::string(pe32.szExeFile) == "java.exe") {
            pids.push_back(pe32.th32ProcessID);
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return pids;
}