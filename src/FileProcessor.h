#ifndef MODINJECTOR_FILEPROCESSOR_H
#define MODINJECTOR_FILEPROCESSOR_H

#pragma once
#include <vector>
#include <string>

class FileProcessor {
public:
    FileProcessor();
    bool checkOrCreateComponentsDir();
    std::vector<std::string> scanCPFiles();
    std::string processSelectedFile(const std::string& filePath);
    [[nodiscard]] std::string getLastError() const;
    [[nodiscard]] std::string getTempDir() const;  // 新增：获取临时目录

    // 允许测试访问verifyCPIntegrity
    bool verifyCPIntegrity(const std::string& cpFilePath);

private:
    std::string componentsDir;
    std::string tempDir;
    std::string lastError;

    bool createDirectory(const std::string& path);
    bool copyFileToTemp(const std::string& source);
    bool renameToZip(const std::string& filePath);
    std::string unzipFile(const std::string& zipPath);
    std::string getTempPath();
};

#endif //MODINJECTOR_FILEPROCESSOR_H