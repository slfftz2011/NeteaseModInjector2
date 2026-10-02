#include "../Injector.h"
#include <iostream>
#include <filesystem>

int main() {
    Injector injector;

    std::cout << "Testing Injector copyDirectory and backupDirectories..." << std::endl;

    // Test copyDirectory with a sample directory (adjust paths as needed)
    std::string source = injector.modsDest;
    std::string dest = injector.modsBackup + "_test";

    bool copyResult = Injector::copyDirectory(source, dest);
    std::cout << "copyDirectory from " << source << " to " << dest << ": " << (copyResult ? "Success" : "Failure") << std::endl;

    // Test backupDirectories
    injector.backupDirectories();

    // Clean up test directory if needed
    if (std::filesystem::exists(dest)) {
        std::filesystem::remove_all(dest);
        std::cout << "Cleaned up test backup directory." << std::endl;
    }

    std::cout << "Injector tests completed." << std::endl;
    return 0;
}
