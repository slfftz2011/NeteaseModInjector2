
#include "RegistryReader.h"
#include <iostream>

int main() {
    RegistryReader reader;
    std::string path = reader.GetNeteaseDownloadPath();

    if (!path.empty()) {
        std::cout << "Netease Download Path: " << path << std::endl;
    } else {
        std::cerr << "Error: " << reader.GetLastError() << std::endl;
    }

    return 0;
}
