#include "FileProcessor.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <windows.h>
#include <shlobj.h>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "JsonParser.h"

namespace fs = std::filesystem;

FileProcessor::FileProcessor() {
    componentsDir = "components";
    tempDir = getTempPath() + "\\MI.tmp";
}

bool FileProcessor::checkOrCreateComponentsDir() {
    if (!fs::exists(componentsDir)) {
        return createDirectory(componentsDir);
    }
    return true;
}

std::vector<std::string> FileProcessor::scanCPFiles() {
    std::vector<std::string> cpFiles;

    if (!fs::exists(componentsDir)) {
        lastError = "Components directory does not exist";
        return cpFiles;
    }

    for (const auto& entry : fs::directory_iterator(componentsDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".COP") {
            cpFiles.push_back(entry.path().filename().string());
        }
    }

    return cpFiles;
}

std::string FileProcessor::processSelectedFile(const std::string& filePath) {
    std::string sourcePath = componentsDir + "\\" + filePath;

    if (!fs::exists(sourcePath)) {
        lastError = "Selected file does not exist";
        return "";
    }

    if (!fs::exists(tempDir) && !createDirectory(tempDir)) {
        return "";
    }

    if (!copyFileToTemp(sourcePath)) {
        return "";
    }

    std::string tempFilePath = tempDir + "\\" + filePath;
    if (!renameToZip(tempFilePath)) {
        return "";
    }

    std::string zipPath = tempFilePath + ".zip";
    std::string actualContentPath = unzipFile(zipPath);
    if (actualContentPath.empty()) {
        return "";
    }

    return actualContentPath;
}

bool FileProcessor::createDirectory(const std::string& path) {
    try {
        if (!fs::exists(path)) {
            if (!fs::create_directories(path)) {
                lastError = "Failed to create directory: " + path;
                return false;
            }
        }
    } catch (const fs::filesystem_error& e) {
        lastError = "Filesystem error: " + std::string(e.what());
        return false;
    }
    return true;
}

bool FileProcessor::copyFileToTemp(const std::string& source) {
    try {
        fs::path sourcePath(source);
        fs::path destPath = fs::path(tempDir) / sourcePath.filename();
        fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
    } catch (const fs::filesystem_error& e) {
        lastError = "Failed to copy file: " + std::string(e.what());
        return false;
    }
    return true;
}

bool FileProcessor::renameToZip(const std::string& filePath) {
    try {
        fs::path oldPath(filePath);
        fs::path newPath = oldPath;
        newPath += ".zip";
        fs::rename(oldPath, newPath);
    } catch (const fs::filesystem_error& e) {
        lastError = "Failed to rename file to zip: " + std::string(e.what());
        return false;
    }
    return true;
}

std::string FileProcessor::unzipFile(const std::string& zipPath) {
    std::string destFolder = tempDir + "\\unzipped";
    if (!createDirectory(destFolder)) {
        return ""; // 失败返回空
    }

#ifdef _WIN32
    std::string command = "powershell -Command \"Expand-Archive -Path '" + zipPath + "' -DestinationPath '" + destFolder + "' -Force\"";
    if (system(command.c_str()) != 0) {
        lastError = "Failed to unzip file using PowerShell";
        return "";
    }
#else
    std::string command = "unzip -o '" + zipPath + "' -d '" + destFolder + "'";
    if (system(command.c_str()) != 0) {
        lastError = "Failed to unzip file using unzip command";
        return "";
    }
#endif

    // 查找解压后的实际内容根目录（处理组件解压后包含子目录的情况）
    std::string actualContentPath = destFolder;
    try {
        // 首先检查根目录是否有list.json，如果有则直接使用根目录
        if (fs::exists(destFolder + "\\list.json")) {
            actualContentPath = destFolder;
        } else {
            // 否则遍历解压目录，寻找第一个子目录（通常是组件根目录）
            for (const auto& entry : fs::directory_iterator(destFolder)) {
                if (entry.is_directory()) {
                    actualContentPath = entry.path().string();
                    break; // 取第一个子目录作为根目录
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        lastError = "Failed to check unzipped content: " + std::string(e.what());
        return "";
    }

    return actualContentPath;
}


std::string FileProcessor::getTempPath() {
    char buffer[MAX_PATH];
    if (GetTempPathA(MAX_PATH, buffer) == 0) {
        return ".";
    }
    return std::string(buffer);
}

std::string FileProcessor::getLastError() const {
    return lastError;
}

// 新增：简单的完整性验证实现（替换OpenSSL依赖）
bool FileProcessor::verifyCPIntegrity(const std::string& cpFilePath) {
    std::string fullPath = componentsDir + "\\" + cpFilePath;
    if (!fs::exists(fullPath)) {
        lastError = "File not found: " + fullPath;
        return false;
    }

    // 简单验证：检查文件大小（实际项目需替换为真实哈希验证）
    try {
        uintmax_t fileSize = fs::file_size(fullPath);
        if (fileSize == 0) {
            lastError = "Empty CP file";
            return false;
        }

        // 读取文件头部判断是否为有效ZIP格式（CP文件本质是ZIP）
        std::ifstream file(fullPath, std::ios::binary);
        if (!file.is_open()) {
            lastError = "Failed to open file";
            return false;
        }

        char header[4];
        file.read(header, 4);
        if (std::string(header, 4) != "PK\x03\x04") {  // ZIP文件标识
            lastError = "Invalid CP file format (not a ZIP archive)";
            return false;
        }
    } catch (const fs::filesystem_error& e) {
        lastError = "Verification failed: " + std::string(e.what());
        return false;
    }

    return true;
}

// 新增：获取临时目录
std::string FileProcessor::getTempDir() const {
    return tempDir;
}

