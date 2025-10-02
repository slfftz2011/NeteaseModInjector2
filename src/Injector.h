#ifndef MODINJECTOR2_INJECTOR_H
#define MODINJECTOR2_INJECTOR_H

#pragma once
#include <fstream>

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

    static bool waitForLogDeletion(const std::string& logPath);

    void backupDirectories();
};


#endif //MODINJECTOR2_INJECTOR_H