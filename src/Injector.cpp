#include "Injector.h"
#include <algorithm>
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <shellapi.h>
#include <filesystem>
#include <fstream>

#include "RegistryReader.h"
#include "GameVersionList.h"

RegistryReader registry_reader;

Injector::Injector() {
    panfu = registry_reader.GetNeteaseDownloadPath();
    modsDest = panfu + "\\Game\\.minecraft\\mods";
    configDest = panfu + "\\Game\\.minecraft\\config";
    resourceDest = panfu + "\\Game\\.minecraft\\resourcepacks";
    logPath = modsDest + "\\JuwLBFt.log";
    modsBackup = panfu + "\\Game\\.minecraft\\mods_backup";
    configBackup = panfu + "\\Game\\.minecraft\\config_backup";
    resourceBackup = panfu + "\\Game\\.minecraft\\resourcepacks_backup";

    const std::filesystem::path neoForgeRoot = std::filesystem::path(panfu).parent_path() / "netease_minecraft_neoforge";
    neoForgeModsDest = (neoForgeRoot / "mods").string();
    neoForgeConfigDest = (neoForgeRoot / "config").string();
    neoForgeLogPath = (std::filesystem::path(neoForgeModsDest) / "JuwLBFt.log").string();
}

// 递归复制目录（包含子目录和文件）
bool Injector::copyDirectory(const std::string& source, const std::string& dest) {
    // 创建目标目录
    if (!CreateDirectoryA(dest.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((source + "\\*").c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        std::string fileName = findData.cFileName;
        if (fileName == "." || fileName == "..") {
            continue;
        }

        std::string srcPath = source + "\\" + fileName;
        std::string destPath = dest + "\\" + fileName;

        // 如果是目录则递归复制
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!copyDirectory(srcPath, destPath)) {
                FindClose(hFind);
                return false;
            }
        }
        // 如果是文件则直接复制
        else {
            if (!CopyFileA(srcPath.c_str(), destPath.c_str(), FALSE)) {
                FindClose(hFind);
                return false;
            }
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);
    return true;
}

bool Injector::deployDirectories(const std::string& sourceRoot) {
    const std::pair<std::string, std::string> directories[] = {
        {sourceRoot + "\\mods", modsDest},
        {sourceRoot + "\\config", configDest},
        {sourceRoot + "\\resourcepacks", resourceDest}
    };

    for (const auto& [source, destination] : directories) {
        try {
            if (!std::filesystem::exists(source)) {
                continue;
            }
        } catch (const std::filesystem::filesystem_error& error) {
            lastError = "检查部署目录失败: " + std::string(error.what());
            return false;
        }

        if (!copyDirectory(source, destination)) {
            lastError = "部署目录失败: " + source + " (Windows error " + std::to_string(GetLastError()) + ")";
            return false;
        }
    }
    return removeAtZeroMods(modsDest);
}

bool Injector::prepareVersionDeployment(const std::string& sourceRoot, const std::string& gameVersion) {
    if (!game_versions::isValid(gameVersion)) {
        lastError = "无效的 Minecraft 版本号: " + gameVersion;
        return false;
    }
    if (game_versions::usesNeoForge(gameVersion)) {
        return true;
    }

    std::string directoryVersion = gameVersion;
    std::replace(directoryVersion.begin(), directoryVersion.end(), '.', '_');
    const std::filesystem::path versionRoot = std::filesystem::path(panfu) / "cache" / "game" / ("V_" + directoryVersion);
    const std::pair<std::string, std::string> directories[] = {
        {sourceRoot + "\\mods", (versionRoot / "mods").string()},
        {sourceRoot + "\\config", (versionRoot / "config").string()}
    };

    try {
        std::filesystem::create_directories(versionRoot);
        for (const auto& [source, destination] : directories) {
            if (!std::filesystem::exists(source)) {
                continue;
            }
            std::filesystem::create_directories(destination);
            if (!copyDirectory(source, destination)) {
                lastError = "预置版本目录失败: " + destination + " (Windows error " + std::to_string(GetLastError()) + ")";
                return false;
            }
        }
    } catch (const std::filesystem::filesystem_error& error) {
        lastError = "创建版本目录失败: " + std::string(error.what());
        return false;
    }

    return removeAtZeroMods((versionRoot / "mods").string());
}

bool Injector::deployNeoForgeDirectories(const std::string& sourceRoot) {
    const std::pair<std::string, std::string> directories[] = {
        {sourceRoot + "\\mods", neoForgeModsDest},
        {sourceRoot + "\\config", neoForgeConfigDest}
    };

    try {
        for (const auto& [source, destination] : directories) {
            if (!std::filesystem::exists(source)) {
                continue;
            }
            std::filesystem::create_directories(destination);
            if (!copyDirectory(source, destination)) {
                lastError = "部署 NeoForge 目录失败: " + destination + " (Windows error " + std::to_string(GetLastError()) + ")";
                return false;
            }
        }
    } catch (const std::filesystem::filesystem_error& error) {
        lastError = "检查 NeoForge 目录失败: " + std::string(error.what());
        return false;
    }

    return removeAtZeroMods(neoForgeModsDest);
}

bool Injector::createLogTrigger(const std::string& path, const std::string& contents) {
    try {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    } catch (const std::filesystem::filesystem_error& error) {
        lastError = "创建日志目录失败: " + std::string(error.what());
        return false;
    }

    std::ofstream logFile(path, std::ios::binary | std::ios::trunc);
    if (!logFile || !(logFile << contents)) {
        lastError = "创建日志触发文件失败: " + path;
        return false;
    }
    return true;
}

bool Injector::hasLogBeenDeleted(const std::string& path) {
    if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    const DWORD error = GetLastError();
    return error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND;
}

// 等待日志文件被删除（最多900秒）
bool Injector::waitForLogDeletion(const std::string& logPath) {
    for (int i = 0; i < 900; ++i) {
        if (hasLogBeenDeleted(logPath)) {
            return true;
        }
        Sleep(1000); // 等待1秒
    }
    return false; // 超时
}

bool Injector::waitForAnyLogDeletion(
    const std::string& primaryLogPath,
    const std::string& neoForgeLogPath,
    bool& primaryDeleted,
    bool& neoForgeDeleted,
    int timeoutSeconds) {
    primaryDeleted = false;
    neoForgeDeleted = false;
    for (int second = 0; second < timeoutSeconds; ++second) {
        primaryDeleted = hasLogBeenDeleted(primaryLogPath);
        neoForgeDeleted = hasLogBeenDeleted(neoForgeLogPath);
        if (primaryDeleted || neoForgeDeleted) {
            return true;
        }
        Sleep(1000);
    }
    return false;
}

// 备份现有目录
bool Injector::backupDirectories() {
    std::cout << "正在备份现有文件...\n";
    const std::pair<std::string, std::string> directories[] = {
        {modsDest, modsBackup},
        {configDest, configBackup},
        {resourceDest, resourceBackup}
    };

    for (const auto& [source, backup] : directories) {
        try {
            if (!std::filesystem::exists(source)) {
                continue;
            }
        } catch (const std::filesystem::filesystem_error& error) {
            lastError = "检查备份目录失败: " + std::string(error.what());
            return false;
        }

        if (!copyDirectory(source, backup)) {
            lastError = "备份目录失败: " + source + " (Windows error " + std::to_string(GetLastError()) + ")";
            return false;
        }
    }

    std::cout << "备份完成\n";
    return true;
}

std::string Injector::getLastError() const {
    return lastError;
}

bool Injector::removeAtZeroMods(const std::string& modsPath) {
    try {
        if (!std::filesystem::exists(modsPath)) {
            return true;
        }
        for (const auto& entry : std::filesystem::directory_iterator(modsPath)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const std::string stem = entry.path().stem().string();
            if (stem.size() < 2 || stem.compare(stem.size() - 2, 2, "@0") != 0) {
                continue;
            }
            std::error_code error;
            std::filesystem::remove(entry.path(), error);
            if (error) {
                lastError = "删除 @0 模组文件失败: " + entry.path().string() + " (" + error.message() + ")";
                return false;
            }
        }
    } catch (const std::filesystem::filesystem_error& error) {
        lastError = "清理 @0 模组文件失败: " + std::string(error.what());
        return false;
    }
    return true;
}
