
#include "URLOpener.h"
#include <iostream>

int main() {
    std::string url;
    std::cout << "Input URL: ";
    std::getline(std::cin, url);

    if (URLOpener::OpenURL(url)) {
        std::cout << "Successfully open URL: " << url << std::endl;
    } else {
        std::cerr << "Error: " << URLOpener::GetLastError() << std::endl;
    }

    return 0;
}
