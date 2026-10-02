
#ifndef MODINJECTOR2_INJECTOR_H
#define MODINJECTOR2_INJECTOR_H


#pragma once
#include <fstream>
#include <string>

class Injector {
public:
    std::string panfu;
    std::string modsDest;
    std::string configDest;
    std::string resourceDest;
    std::string logPath;
    std::string neoForgeLogPath;
    std::string modsBackup;
    std::string configBackup;
    std::string resourceBackup;

    Injector();

    static bool copyDirectory(const std::string& source, const std::string& dest);
    bool deployDirectories(const std::string& sourceRoot);
    bool prepareVersionDeployment(const std::string& sourceRoot, const std::string& gameVersion);
    bool deployNeoForgeDirectories(const std::string& sourceRoot);
    bool createLogTrigger(const std::string& path, const std::string& contents);
    static bool hasLogBeenDeleted(const std::string& path);

    static bool waitForLogDeletion(const std::string& logPath);
    static bool waitForAnyLogDeletion(
        const std::string& primaryLogPath,
        const std::string& neoForgeLogPath,
        bool& primaryDeleted,
        bool& neoForgeDeleted,
        int timeoutSeconds = 900);

    bool backupDirectories();
    std::string getLastError() const;

private:
    std::string lastError;
    std::string neoForgeModsDest;
    std::string neoForgeConfigDest;
    bool removeAtZeroMods(const std::string& modsPath);
};


#endif //MODINJECTOR2_INJECTOR_H