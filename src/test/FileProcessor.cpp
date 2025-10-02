#include "../FileProcessor.h"
#include <iostream>

int main() {
    FileProcessor processor;

    // 1. 检查/创建components目录
    if (!processor.checkOrCreateComponentsDir()) {
        std::cerr << "Error: " << processor.getLastError() << std::endl;
        return 1;
    }

    // 2. 扫描CP文件
    auto cpFiles = processor.scanCPFiles();
    if (cpFiles.empty()) {
        std::cout << "No .COP files found in components directory." << std::endl;
        return 0;
    }

    // 显示文件列表供用户选择
    std::cout << "Available .COP files:" << std::endl;
    for (size_t i = 0; i < cpFiles.size(); ++i) {
        std::cout << i + 1 << ". " << cpFiles[i] << std::endl;
    }

    std::cout << "Select a file (1-" << cpFiles.size() << "): ";
    size_t choice;
    std::cin >> choice;

    if (choice < 1 || choice > cpFiles.size()) {
        std::cerr << "Invalid selection." << std::endl;
        return 1;
    }

    // 3. 验证文件完整性
    std::cout << "Verifying " << cpFiles[choice-1] << "..." << std::endl;
    if (!processor.verifyCPIntegrity(cpFiles[choice-1])) {
        std::cerr << "Integrity check failed: " << processor.getLastError() << std::endl;
        return 1;
    }
    std::cout << "Integrity check passed!" << std::endl;

    // 4. 处理选中的文件
    if (processor.processSelectedFile(cpFiles[choice - 1])) {
        std::cout << "File processed successfully! Temp dir: " << processor.getTempDir() << std::endl;
    } else {
        std::cerr << "Error: " << processor.getLastError() << std::endl;
        return 1;
    }

    return 0;
}