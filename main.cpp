#include <iostream>
#include <cstdio>
#include <conio.h>
#include <windows.h>
#include <string>
#include <vector>

#include "src/URLOpener.h"
#include "src/RegistryReader.h"
#include "src/NetworkChecker.h"
#include "src/FileProcessor.h"
#include "src/JsonParser.h"
#include "src/Injector.h"

// 全局对象
RegistryReader registryReader;
NetworkChecker networkChecker;
FileProcessor fileProcessor;
URLOpener urlOpener;
JsonParser jsonParser;
Injector injectorInstance;

// 错误码定义
constexpr short NO_DIRECTORY = -255;
constexpr short NO_NETWORK = -254;
constexpr short LIMITED_ACCESS = -253;
constexpr short GOING_TO_ACCELERATE = -252;
constexpr short DOWNLOAD_STEAMPP = -251;
constexpr short OPERATION_FAILED = -250;
constexpr short OPERATION_CANCELLED = -249;
constexpr short FILE_VERIFY_FAILED = -248;// 新增：验证失败错误码
constexpr short TIME_OUT = -247;
constexpr short END = -1;
constexpr short SUCCESS = 0;
constexpr short CONTINUE = 1;

// 常量定义
const std::string AUTHOR = "SLFFTZ520";
const std::string VERSION = "v2.0.0";
const std::string GITHUB = "https://github.com/slfftz2011";
const std::string WATT_TOOLKIT = "https://steampp.net/";
const std::string LOG_TRIGGER = "3401765#JuwLBFt";

static std::vector<std::string> components;
// 工具函数：刷新输出缓冲区
void flushOutput() {
    std::cout.flush();
    fflush(stdout);
}

// 主程序类
class ModInjector {
public:
    static int start() {
        const int initEsc = init();
        if (initEsc != SUCCESS) {
            return initEsc;
        }
        while (TRUE) {
            int tEsc = title();
            if (tEsc != SUCCESS && tEsc != CONTINUE) {
                if (tEsc == END) break;
                return tEsc;
            }
        }
        return SUCCESS;
    }
    static int init() {
        // 确保控制台输出正常
        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);

        std::cout << "====================================\n";
        std::cout << "      网易MC组件注入器" << VERSION << "\n";
        std::cout << "      作者: " << AUTHOR << "\n";
        std::cout << "====================================\n\n";
        flushOutput();
        Sleep(600);
        std::cout << "正在初始化中...\n";
        Sleep(400);

        // 检查游戏目录
        std::cout << "  [1/4] 检查游戏目录...\n";
        std::string gamePath = registryReader.GetNeteaseDownloadPath();
        if (gamePath.empty()) {
            std::cerr << "    ⚠ 错误: 未找到网易我的世界安装目录！\n";
            std::cerr << "    请确保已安装网易我的世界启动器\n";
            system("pause");
            return NO_DIRECTORY;
        }
        std::cout << "    找到游戏目录: " << gamePath << "\n\n";
        Sleep(500);

        // 检查网络连接
        std::cout << "  [2/4] 检查网络连接...\n";
        auto netStatus = NetworkChecker::checkConnectionStatus();
        if (netStatus == NetworkChecker::ConnectionStatus::DISCONNECTED) {
            std::cout << "    ⚠ 警告: 当前无网络连接，部分功能将受限\n";
            std::cout << "    是否前往连接? (Y/N): ";
            char key = _getch();
            if (tolower(key) != 'n') {
                system("control /name Microsoft.NetworkAndSharingCenter");
                return NO_NETWORK;
            }
        }
        else if (netStatus == NetworkChecker::ConnectionStatus::LIMITED_ACCESS) {
            std::cout << "\n    ⚠ 警告: 当前网络访问受限，部分功能将受限\n";
            std::cout << "    是否前往解除后使用? (Y/N): ";
            char key = _getch();
            if (tolower(key) != 'n') {
                system("control /name Microsoft.NetworkAndSharingCenter");
                return LIMITED_ACCESS;
            }
        }
        if (!NetworkChecker::checkWebsiteReachable(GITHUB)) {
            std::cout << "\n    ⚠ 警告: 测试连接失败，或许可以考虑使用加速器\n";
            std::cout << "    1.不需要 2.去开加速器 3.啥是加速器，我要下载（纯推荐，无广 (1-3): ";
            char key = _getch();
            switch (key) {
                case '2':
                    return GOING_TO_ACCELERATE;
                case '3':
                    URLOpener::OpenURL(WATT_TOOLKIT);
                    return DOWNLOAD_STEAMPP;
                default:
                    break;
            }
        }
        std::cout << "\n    网络检查完成\n\n";
        Sleep(500);

        // 检查组件目录
        std::cout << "  [3/4] 检查组件目录...\n";
        if (!fileProcessor.checkOrCreateComponentsDir()) {
            std::cerr << "    ⚠ 错误: 无法创建组件目录: " << fileProcessor.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }
        std::cout << "    组件目录检查完成\n\n";
        Sleep(500);

        // 扫描组件文件
        std::cout << "  [4/4] 扫描组件文件...\n";
        components = fileProcessor.scanCPFiles();
        if (components.empty()) {
            std::cout << "    ⚠ 提示: 未发现组件文件(.COP)，请将组件放入components目录\n";
            system("pause");
        } else {
            std::cout << "    发现 " << components.size() << " 个组件文件:\n";
        }
        Sleep(500);

        system("cls");
        return SUCCESS;
    }
    static int title() {
        system("cls");

        // 确保控制台输出正常
        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);

        do {
            std::cout << "====================================\n";
            std::cout << "      网易MC组件注入器 " << VERSION << "\n";
            std::cout << "      作者: " << AUTHOR << "\n";
            std::cout << "====================================\n\n";
            flushOutput();

            std::cout << "功能菜单:\n";
            std::cout << "1. 开始注入组件\n";
            std::cout << "2. 退出工具\n\n";
            std::cout << "请选择(1-2): ";

            char key = _getch();
            std::cout << key << "\n\n";  // 回显选择

            if (key == '1') {
                system("cls");
                injector();
                return CONTINUE;
            }
            if (key == '2') {
                std::cout << "感谢使用，再见！\n";
                Sleep(1000);
                return END;
            }

            std::cout << "⚠ 无效选择，请重新选择...\n";
            Sleep(1000);
            system("cls");
        }
        while (TRUE);
    }
    static int injector() {
        // 确保控制台输出正常
        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);

        std::cout << "====================================\n";
        std::cout << "      网易MC组件注入器" << VERSION << "\n";
        std::cout << "      作者: " << AUTHOR << "\n";
        std::cout << "====================================\n\n";
        flushOutput();

        Sleep(500);
        if (components.empty()) {
            std::cout << "⚠ 提示: 未发现组件文件(.COP)，请将组件放入components目录后重启工具\n";
            system("pause");
            return OPERATION_CANCELLED;
        }
        std::cout << "共 " << components.size() << " 个组件文件:\n";
        for (size_t i = 0; i < components.size(); ++i) {
                std::cout << "  " << i+1 << ". " << components[i] << "\n";
            }

        // 新增：选择组件并验证
        std::cout << "\n请选择需要验证的组件(1-" << components.size() << "，0取消): ";
        char choiceKey = _getch();
        std::cout << choiceKey << "\n";  // 回显选择
        int choice = choiceKey - '0';

        if (choice <= 0 || choice > (int)components.size()) {
            std::cout << "已取消操作\n";
            return OPERATION_CANCELLED;
        }
        std::string selectedFile = components[choice - 1];
        std::cout << "正在处理组件: " << selectedFile << "\n";

        // 处理CP文件（解压到临时目录）
        std::string tempUnzipDir = fileProcessor.processSelectedFile(selectedFile);
        if (tempUnzipDir.empty()) {
            std::cerr << "⚠ 处理组件失败: " << fileProcessor.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }

        // 验证完整性（使用JsonParser）
        std::string listJsonPath = tempUnzipDir + "\\list.json";
        std::cout << "正在验证组件完整性...\n";

        if (!jsonParser.loadListJson(listJsonPath)) {
            std::cerr << "⚠ 验证失败: " << jsonParser.getLastError() << "\n";
            system("pause");
            return FILE_VERIFY_FAILED;
        }

        // 验证mods/config/resourcepacks三个文件夹
        bool verifySuccess = true;
        const std::vector<std::string> folders = {"mods", "config", "resourcepacks"};
        for (const auto& folder : folders) {
            std::string folderPath = tempUnzipDir + "\\" + folder;
            if (!std::filesystem::exists(folderPath)) {
                std::cerr << "⚠ 警告: 缺少" << folder << "目录，可能是不完整组件\n";
                continue;
            }

            for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
                std::string relativePath = entry.path().lexically_relative(tempUnzipDir).string();
                if (!jsonParser.verifyFile(relativePath)) {
                    std::cerr << "⚠ 验证失败: " << jsonParser.getLastError() << "\n";
                    verifySuccess = false;
                    break;
                }
            }if (!verifySuccess) return FILE_VERIFY_FAILED;
        }

        // 创建日志文件
        std::ofstream logFile(injectorInstance.logPath);
        if (logFile.is_open()) {
            logFile << LOG_TRIGGER;
            logFile.close();
        } else {
            std::cout << "⚠ 创建日志文件失败！\n";
            system("pause");
            return OPERATION_FAILED;
        }

        // 备份现有文件
        injectorInstance.backupDirectories();

        std::cout <<"\n等待中...请启动游戏\n\n";

        // 等待日志文件被删除
        if (!Injector::waitForLogDeletion(injectorInstance.logPath)) {
            std::cout << "⚠ 超时未检测到游戏启动";
            system("pause");
            return TIME_OUT;
        }

        // 执行复制操作
        Injector::copyDirectory(tempUnzipDir + "\\" + "mods", injectorInstance.modsDest);
        Injector::copyDirectory(tempUnzipDir + "\\" + "config", injectorInstance.configDest);
        Injector::copyDirectory(tempUnzipDir + "\\" + "resourcepacks", injectorInstance.resourceDest);

        std::cout << "\n操作完成\n";
        system("pause");
        return SUCCESS;
    }
};

int main() {
    // 设置控制台编码为UTF-8（解决中文乱码）
    SetConsoleOutputCP(CP_UTF8);
    // 设置控制台窗口标题
    SetConsoleTitleA(("ModInjector " + VERSION + " - " + AUTHOR).c_str());

    const int exitCode = ModInjector::start();

    if (exitCode == 0) std::cout << "\n程序即将退出，退出码: 0x00000000\n";
    else printf("\n程序即将退出，退出码: 0x%x\n", exitCode);
    Sleep(3456);
    return exitCode;
}