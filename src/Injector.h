
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
    std::string modsBackup;
    std::string configBackup;
    std::string resourceBackup;

    Injector();

    static bool copyDirectory(const std::string& source, const std::string& dest);
    bool deployDirectories(const std::string& sourceRoot);

    static bool waitForLogDeletion(const std::string& logPath);

    bool backupDirectories();
    std::string getLastError() const;

private:
    std::string lastError;
};


#endif //MODINJECTOR2_INJECTOR_H