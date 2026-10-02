#include "Application.h"

#include <charconv>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <windows.h>

#include "FileProcessor.h"
#include "GameVersionList.h"
#include "Injector.h"
#include "JsonParser.h"
#include "MciApiClient.h"
#include "NetworkChecker.h"
#include "RegistryReader.h"
#include "URLOpener.h"
#include "ConsoleOutput.h"

namespace {
constexpr short NO_DIRECTORY = -255;
constexpr short NO_NETWORK = -254;
constexpr short LIMITED_ACCESS = -253;
constexpr short GOING_TO_ACCELERATE = -252;
constexpr short DOWNLOAD_STEAMPP = -251;
constexpr short OPERATION_FAILED = -250;
constexpr short OPERATION_CANCELLED = -249;
constexpr short FILE_VERIFY_FAILED = -248;
constexpr short TIME_OUT = -247;
constexpr short END = -1;
constexpr short SUCCESS = 0;
constexpr short CONTINUE = 1;

const std::string AUTHOR = "SLFFTZ520";
const std::string VERSION = PROJECT_VERSION_FULL;
const std::string GITHUB = "https://github.com/slfftz2011";
const std::string WATT_TOOLKIT = "https://steampp.net/";
const std::string LOG_TRIGGER = "3401765#JuwLBFt";

void flushOutput() {
    std::cout.flush();
    std::fflush(stdout);
}

class Application {
public:
    int start() {
        const int initResult = init();
        if (initResult != SUCCESS) {
            return initResult;
        }

        while (true) {
            const int menuResult = title();
            if (menuResult != SUCCESS && menuResult != CONTINUE) {
                return menuResult == END ? SUCCESS : menuResult;
            }
        }
    }

private:
    RegistryReader registryReader;
    FileProcessor fileProcessor;
    JsonParser jsonParser;
    Injector injectorInstance;
    MciApiClient mciClient;
    std::vector<std::string> components;

    int init() {
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

        std::cout << "  [1/4] 检查游戏目录...\n";
        const std::string gamePath = registryReader.GetNeteaseDownloadPath();
        if (gamePath.empty()) {
            std::cerr << "    [错误] 未找到网易我的世界安装目录！\n";
            std::cerr << "    请确保已安装网易我的世界启动器\n";
            system("pause");
            return NO_DIRECTORY;
        }
        std::cout << "    找到游戏目录: " << gamePath << "\n\n";
        Sleep(500);

        std::cout << "  [2/4] 检查网络连接...\n";
        const auto networkStatus = NetworkChecker::checkConnectionStatus();
        if (networkStatus == NetworkChecker::ConnectionStatus::DISCONNECTED) {
            std::cout << "    [警告] 当前无网络连接，部分功能将受限\n";
            std::cout << "    是否前往连接? (Y/N): ";
            const char key = _getch();
            if (tolower(key) != 'n') {
                system("control /name Microsoft.NetworkAndSharingCenter");
                return NO_NETWORK;
            }
        } else if (networkStatus == NetworkChecker::ConnectionStatus::LIMITED_ACCESS) {
            std::cout << "\n    [警告] 当前网络访问受限，部分功能将受限\n";
            std::cout << "    是否前往解除后使用? (Y/N): ";
            const char key = _getch();
            if (tolower(key) != 'n') {
                system("control /name Microsoft.NetworkAndSharingCenter");
                return LIMITED_ACCESS;
            }
        }

        if (!NetworkChecker::checkWebsiteReachable(GITHUB)) {
            std::cout << "\n    [警告] 测试连接失败，或许可以考虑使用加速器/本地代理\n";
            std::cout << "    1.不需要 2.去开加速器 3.了解Watt Toolkit并前往下载 (1-3): ";
            const char key = _getch();
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

        std::cout << "  [3/4] 检查组件目录...\n";
        if (!fileProcessor.checkOrCreateComponentsDir()) {
            std::cerr << "    [错误] 无法创建组件目录: " << fileProcessor.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }
        std::cout << "    组件目录检查完成\n\n";
        Sleep(500);

        std::cout << "  [4/4] 扫描组件文件...\n";
        components = fileProcessor.scanCPFiles();
        if (components.empty()) {
            std::cout << "    [提示] 未发现组件文件(.COP)，请将组件放入components目录\n";
            system("pause");
        } else {
            std::cout << "    发现 " << components.size() << " 个组件文件:\n";
        }
        Sleep(500);
        system("cls");
        return SUCCESS;
    }

    int title() {
        system("cls");
        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);

        while (true) {
            std::cout << "====================================\n";
            std::cout << "      网易MC组件注入器 " << VERSION << "\n";
            std::cout << "      作者: " << AUTHOR << "\n";
            std::cout << "====================================\n\n";
            flushOutput();

            std::cout << "功能菜单:\n";
            std::cout << "1. 开始注入组件\n";
            std::cout << "2. 从 MCI 搜索并下载模组 (Modrinth/CurseForge)\n";
            std::cout << "3. 退出工具\n\n";
            std::cout << "请选择(1-3): ";

            const char key = _getch();
            std::cout << key << "\n\n";
            if (key == '1') {
                system("cls");
                injector();
                return CONTINUE;
            }
            if (key == '2') {
                system("cls");
                downloadFromMci();
                return CONTINUE;
            }
            if (key == '3') {
                std::cout << "感谢使用，再见！\n";
                Sleep(1000);
                return END;
            }

            std::cout << "[警告] 无效选择，请重新选择...\n";
            Sleep(1000);
            system("cls");
        }
    }

    int readSelection(const std::string& prompt, int maximum) {
        std::cout << prompt;
        std::string input;
        readUtf8Line(input);
        int selection = -1;
        const auto result = std::from_chars(input.data(), input.data() + input.size(), selection);
        if (result.ec != std::errc{} || result.ptr != input.data() + input.size() ||
            selection < 0 || selection > maximum) {
            return -1;
        }
        return selection;
    }

    std::string chooseGameVersion() {
        std::cout << "目标 Minecraft 版本:\n";
        for (size_t index = 0; index < game_versions::supported.size(); ++index) {
            const auto version = game_versions::supported[index];
            std::cout << index + 1 << ". " << version;
            if (game_versions::usesNeoForge(version)) {
                std::cout << " (NeoForge)";
            }
            std::cout << "\n";
        }
        std::cout << "0. 输入其他版本\n";
        const int selection = readSelection("选择版本: ", static_cast<int>(game_versions::supported.size()));
        if (selection < 0) {
            return {};
        }
        if (selection == 0) {
            std::string version;
            std::cout << "输入版本号 (例如 1.20.1): ";
            readUtf8Line(version);
            return game_versions::isValid(version) ? version : std::string{};
        }
        return std::string(game_versions::supported[static_cast<size_t>(selection - 1)]);
    }

    int downloadFromMci() {
        const int platform = readSelection("平台: 1. Modrinth  2. CurseForge  0.取消: ", 2);
        if (platform <= 0) {
            return OPERATION_CANCELLED;
        }

        std::string query;
        std::string gameVersion;
        std::string loader;
        std::cout << "搜索词: ";
        readUtf8Line(query);
        if (query.empty()) {
            std::cout << "搜索词不能为空。\n";
            system("pause");
            return OPERATION_CANCELLED;
        }
        std::cout << "Minecraft 版本 (留空不筛选): ";
        readUtf8Line(gameVersion);
        std::cout << "加载器 fabric/forge/neoforge/quilt (留空不筛选): ";
        readUtf8Line(loader);

        if (platform == 2) {
            return downloadCurseForge(query, gameVersion, loader);
        }

        std::cout << "正在查询 MCI...\n";
        const auto projects = mciClient.searchMods(query, gameVersion, loader);
        if (projects.empty()) {
            std::cerr << "MCI 搜索失败或没有结果: " << mciClient.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }
        for (size_t index = 0; index < projects.size(); ++index) {
            std::cout << index + 1 << ". " << projects[index].title << " by "
                      << projects[index].author << " (" << projects[index].downloads << " downloads)\n";
        }
        int selection = readSelection("选择项目 (0取消): ", static_cast<int>(projects.size()));
        if (selection <= 0) {
            return OPERATION_CANCELLED;
        }

        const MciProject& project = projects[static_cast<size_t>(selection - 1)];
        const auto versions = mciClient.getProjectVersions(project.id, gameVersion, loader);
        if (versions.empty()) {
            std::cerr << "没有匹配版本: " << mciClient.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }
        for (size_t index = 0; index < versions.size(); ++index) {
            std::cout << index + 1 << ". " << versions[index].number << " ("
                      << versions[index].files.size() << " files)\n";
        }
        selection = readSelection("选择版本 (0取消): ", static_cast<int>(versions.size()));
        if (selection <= 0) {
            return OPERATION_CANCELLED;
        }

        const MciVersion& version = versions[static_cast<size_t>(selection - 1)];
        size_t fileIndex = 0;
        if (version.files.size() > 1) {
            for (size_t index = 0; index < version.files.size(); ++index) {
                std::cout << index + 1 << ". " << version.files[index].fileName
                          << (version.files[index].primary ? " [primary]" : "") << "\n";
            }
            selection = readSelection("选择文件 (0取消): ", static_cast<int>(version.files.size()));
            if (selection <= 0) {
                return OPERATION_CANCELLED;
            }
            fileIndex = static_cast<size_t>(selection - 1);
        } else if (version.files.empty()) {
            std::cerr << "所选版本没有可下载文件。\n";
            system("pause");
            return OPERATION_FAILED;
        }

        std::string savedPath;
        if (!mciClient.downloadFile(project.id, version.id, version.files[fileIndex].fileName, savedPath)) {
            std::cerr << "下载失败: " << mciClient.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }
        std::cout << "下载完成: " << savedPath << "\n";
        system("pause");
        return SUCCESS;
    }

    int downloadCurseForge(
        const std::string& query,
        const std::string& gameVersion,
        const std::string& loader) {
        std::cout << "正在查询 MCI CurseForge...\n";
        const auto projects = mciClient.searchCurseForgeMods(query, gameVersion);
        if (projects.empty()) {
            std::cerr << "CurseForge 搜索失败或没有结果: " << mciClient.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }
        for (size_t index = 0; index < projects.size(); ++index) {
            std::cout << index + 1 << ". " << projects[index].title << " by "
                      << projects[index].author << " (" << projects[index].downloads << " downloads)\n";
        }
        int selection = readSelection("选择项目 (0取消): ", static_cast<int>(projects.size()));
        if (selection <= 0) {
            return OPERATION_CANCELLED;
        }

        const MciCurseForgeProject& project = projects[static_cast<size_t>(selection - 1)];
        const auto files = mciClient.getCurseForgeFiles(project.id, gameVersion, loader);
        if (files.empty()) {
            std::cerr << "没有匹配文件: " << mciClient.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }

        size_t fileIndex = 0;
        if (files.size() > 1) {
            for (size_t index = 0; index < files.size(); ++index) {
                const std::string& label = files[index].displayName.empty()
                    ? files[index].fileName
                    : files[index].displayName;
                std::cout << index + 1 << ". " << label << " [" << files[index].fileName << "]\n";
            }
            selection = readSelection("选择文件 (0取消): ", static_cast<int>(files.size()));
            if (selection <= 0) {
                return OPERATION_CANCELLED;
            }
            fileIndex = static_cast<size_t>(selection - 1);
        }

        std::string savedPath;
        const auto& file = files[fileIndex];
        if (!mciClient.downloadCurseForgeFile(file.id, file.fileName, savedPath)) {
            std::cerr << "CurseForge 下载失败: " << mciClient.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }
        std::cout << "下载完成: " << savedPath << "\n";
        system("pause");
        return SUCCESS;
    }

    int injector() {
        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);

        std::cout << "====================================\n";
        std::cout << "      网易MC组件注入器" << VERSION << "\n";
        std::cout << "      作者: " << AUTHOR << "\n";
        std::cout << "====================================\n\n";
        flushOutput();

        Sleep(500);
        if (components.empty()) {
            std::cout << "[提示] 未发现组件文件(.COP)，请将组件放入components目录后重启工具\n";
            system("pause");
            return OPERATION_CANCELLED;
        }
        std::cout << "共 " << components.size() << " 个组件文件:\n";
        for (size_t i = 0; i < components.size(); ++i) {
            std::cout << "  " << i + 1 << ". " << components[i] << "\n";
        }

        std::cout << "\n请选择需要验证的组件(1-" << components.size() << "，0取消): ";
        std::string choiceInput;
        readUtf8Line(choiceInput);
        std::cout << choiceInput << "\n";

        int choice = 0;
        const auto parseResult = std::from_chars(
            choiceInput.data(), choiceInput.data() + choiceInput.size(), choice);
        if (parseResult.ec != std::errc{} || parseResult.ptr != choiceInput.data() + choiceInput.size()) {
            choice = 0;
        }

        if (choice <= 0 || choice > static_cast<int>(components.size())) {
            std::cout << "已取消操作\n";
            return OPERATION_CANCELLED;
        }
        const std::string selectedFile = components[choice - 1];
        std::cout << "正在处理组件: " << selectedFile << "\n";

        const std::string tempUnzipDir = fileProcessor.processSelectedFile(selectedFile);
        if (tempUnzipDir.empty()) {
            std::cerr << "[错误] 处理组件失败: " << fileProcessor.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }

        const std::string listJsonPath = tempUnzipDir + "\\list.json";
        std::cout << "正在验证组件完整性...\n";
        if (!jsonParser.loadListJson(listJsonPath)) {
            std::cerr << "[错误] 验证失败: " << jsonParser.getLastError() << "\n";
            system("pause");
            return FILE_VERIFY_FAILED;
        }

        const std::string gameVersion = chooseGameVersion();
        if (gameVersion.empty()) {
            std::cout << "未选择有效版本，已取消。\n";
            return OPERATION_CANCELLED;
        }

        bool verifySuccess = true;
        const std::vector<std::string> folders = {"mods", "config", "resourcepacks"};
        for (const auto& folder : folders) {
            const std::string folderPath = tempUnzipDir + "\\" + folder;
            if (!std::filesystem::exists(folderPath)) {
                std::cerr << "[警告] 缺少" << folder << "目录，可能是不完整组件\n";
                continue;
            }

            for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath)) {
                if (!entry.is_regular_file()) {
                    continue;
                }

                const std::string relativePath = entry.path().lexically_relative(tempUnzipDir).generic_string();
                if (!jsonParser.verifyFile(relativePath)) {
                    std::cerr << "[错误] 验证失败: " << jsonParser.getLastError() << "\n";
                    verifySuccess = false;
                    break;
                }
            }
            if (!verifySuccess) {
                return FILE_VERIFY_FAILED;
            }
        }

        if (!injectorInstance.prepareVersionDeployment(tempUnzipDir, gameVersion)) {
            std::cerr << "[错误] " << injectorInstance.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }

        if (!injectorInstance.backupDirectories()) {
            std::cerr << "[错误] " << injectorInstance.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }

        if (!injectorInstance.createLogTrigger(injectorInstance.logPath, LOG_TRIGGER) ||
            !injectorInstance.createLogTrigger(injectorInstance.neoForgeLogPath, LOG_TRIGGER)) {
            std::cerr << "[错误] " << injectorInstance.getLastError() << "\n";
            system("pause");
            return OPERATION_FAILED;
        }

        std::cout << "\n等待游戏删除对应目录中的日志标记...请启动游戏\n\n";
        bool primaryDeleted = false;
        bool neoForgeDeleted = false;
        if (!Injector::waitForAnyLogDeletion(
                injectorInstance.logPath,
                injectorInstance.neoForgeLogPath,
                primaryDeleted,
                neoForgeDeleted)) {
            std::cout << "[错误] 超时未检测到游戏启动\n";
            system("pause");
            return TIME_OUT;
        }

        bool deployed = true;
        if (primaryDeleted) {
            deployed = injectorInstance.deployDirectories(tempUnzipDir);
        }
        if (deployed && neoForgeDeleted) {
            deployed = injectorInstance.deployNeoForgeDirectories(tempUnzipDir);
        }
        if (!deployed) {
            std::cerr << "[错误] " << injectorInstance.getLastError() << "\n";
            std::cerr << "请检查游戏目录权限和磁盘空间。备份位于游戏目录的 *_backup 文件夹。\n";
            system("pause");
            return OPERATION_FAILED;
        }

        std::cout << "\n操作完成\n";
        system("pause");
        return SUCCESS;
    }
};
}

int runApplication() {
    Application application;
    return application.start();
}