#include "Injector.h"
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <shellapi.h>

#include "RegistryReader.h"

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

// 等待日志文件被删除（最多900秒）
bool Injector::waitForLogDeletion(const std::string& logPath) {
    for (int i = 0; i < 900; ++i) {
        // 检查文件是否存在
        if (GetFileAttributesA(logPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
            return true; // 文件已删除
        }
        Sleep(1000); // 等待1秒
    }
    return false; // 超时
}

// 备份现有目录
void Injector::backupDirectories() {
    std::cout << "正在备份现有文件...\n";
    copyDirectory(modsDest, modsBackup);
    copyDirectory(configDest, configBackup);
    copyDirectory(resourceDest, resourceBackup);
    std::cout << "备份完成\n";
}
