#include <iostream>
#include <windows.h>

#include "src/Application.h"
#include "src/ConsoleOutput.h"

int main() {
    initializeConsoleOutput();
    SetConsoleTitleA("ModInjector2");

    const int exitCode = runApplication();
    if (exitCode == 0) {
        std::cout << "\n\n程序即将退出，退出码: 0x00000000\n";
    } else {
        std::cout << "\n\n程序即将退出，退出码: 0x" << std::hex << exitCode << std::dec << "\n";
    }
    Sleep(3456);
    return exitCode;
}
