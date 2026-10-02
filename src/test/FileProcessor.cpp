#include "../FileProcessor.h"
#include <iostream>

void testScanCPFiles() {
    FileProcessor fp;
    if (!fp.checkOrCreateComponentsDir()) {
        std::cerr << "Failed to create components directory: " << fp.getLastError() << std::endl;
        return;
    }
    auto files = fp.scanCPFiles();
    std::cout << "Found " << files.size() << " .COP files in components directory." << std::endl;
    for (const auto& f : files) {
        std::cout << "  " << f << std::endl;
    }
}

void testVerifyCPIntegrity() {
    FileProcessor fp;
    auto files = fp.scanCPFiles();
    if (files.empty()) {
        std::cerr << "No .COP files found for integrity test." << std::endl;
        return;
    }
    for (const auto& f : files) {
        bool valid = fp.verifyCPIntegrity(f);
        std::cout << "File " << f << " integrity: " << (valid ? "PASS" : "FAIL") << std::endl;
        if (!valid) {
            std::cerr << "Error: " << fp.getLastError() << std::endl;
        }
    }
}

int main() {
    std::cout << "Running FileProcessor tests..." << std::endl;
    testScanCPFiles();
    testVerifyCPIntegrity();
    std::cout << "FileProcessor tests completed." << std::endl;
    return 0;
}
