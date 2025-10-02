
#include "NetworkChecker.h"
#include <iostream>

int main() {
    NetworkChecker checker;

    std::cout << "Network Status: ";
    switch(checker.checkConnectionStatus()) {
        case NetworkChecker::ConnectionStatus::CONNECTED:
            std::cout << "Connected\n"; break;
        case NetworkChecker::ConnectionStatus::LIMITED_ACCESS:
            std::cout << "Limited Access\n"; break;
        default:
            std::cout << "Disconnected\n";
    }

    std::cout << "Google reachable: "
              << (checker.checkWebsiteReachable("www.google.com") ? "Yes" : "No")
              << std::endl;

    auto interfaces = checker.getNetworkInterfaces();
    if (!interfaces.empty()) {
        std::cout << "\nNetwork Interfaces:\n";
        for (const auto& iface : interfaces) {
            std::cout << "- " << iface << "\n";
        }
    }

    return 0;
}
