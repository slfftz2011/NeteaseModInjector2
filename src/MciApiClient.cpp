#include "MciApiClient.h"

#include <charconv>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include <wininet.h>

#include "json.hpp"

namespace {
using Json = nlohmann::json;

class InternetHandle {
public:
    explicit InternetHandle(HINTERNET handle) : handle_(handle) {}
    ~InternetHandle() {
        if (handle_) {
            InternetCloseHandle(handle_);
        }
    }
    InternetHandle(const InternetHandle&) = delete;
    InternetHandle& operator=(const InternetHandle&) = delete;
    HINTERNET get() const { return handle_; }

private:
    HINTERNET handle_;
};

bool hasString(const Json& object, const char* key) {
    return object.is_object() && object.contains(key) && object[key].is_string();
}

bool containsString(const Json& array, const std::string& value) {
    if (!array.is_array()) {
        return false;
    }
    for (const auto& item : array) {
        if (item.is_string() && item.get<std::string>() == value) {
            return true;
        }
    }
    return false;
}

std::string curseForgeLoaderId(const std::string& loader) {
    std::string normalized = loader;
    for (char& character : normalized) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    if (normalized == "forge") return "1";
    if (normalized == "fabric") return "4";
    if (normalized == "quilt") return "5";
    if (normalized == "neoforge") return "6";
    return {};
}
}

std::string MciApiClient::get(const std::string& url) {
    HINTERNET rawSession = InternetOpenA(
        "ModInjector2/2.0", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!rawSession) {
        lastError = "无法初始化 WinINet: " + std::to_string(GetLastError());
        return {};
    }
    InternetHandle session(rawSession);

    HINTERNET rawRequest = InternetOpenUrlA(
        session.get(), url.c_str(), nullptr, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!rawRequest) {
        lastError = "请求 MCI API 失败: " + std::to_string(GetLastError());
        return {};
    }
    InternetHandle request(rawRequest);

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    if (!HttpQueryInfoA(request.get(), HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                        &statusCode, &statusSize, nullptr) || statusCode < 200 || statusCode >= 300) {
        lastError = "MCI API 返回 HTTP 状态: " + std::to_string(statusCode);
        return {};
    }

    std::string response;
    char buffer[8192];
    DWORD bytesRead = 0;
    while (true) {
        if (!InternetReadFile(request.get(), buffer, sizeof(buffer), &bytesRead)) {
            lastError = "读取 MCI API 响应失败: " + std::to_string(GetLastError());
            return {};
        }
        if (bytesRead == 0) {
            break;
        }
        response.append(buffer, bytesRead);
    }
    return response;
}

std::vector<MciProject> MciApiClient::searchMods(
    const std::string& query,
    const std::string& gameVersion,
    const std::string& loader) {
    lastError.clear();
    Json facetGroups = Json::array();
    facetGroups.push_back(Json::array({"project_type:mod"}));
    if (!gameVersion.empty()) {
        facetGroups.push_back(Json::array({"versions:" + gameVersion}));
    }
    if (!loader.empty()) {
        facetGroups.push_back(Json::array({"categories:" + loader}));
    }

    const std::string url = "https://mod.mcimirror.top/modrinth/v2/search?query=" +
        encodePathSegment(query) + "&limit=20&facets=" + encodePathSegment(facetGroups.dump());
    const std::string body = get(url);
    if (body.empty()) {
        return {};
    }

    try {
        const Json response = Json::parse(body);
        if (!response.contains("hits") || !response["hits"].is_array()) {
            lastError = "MCI 搜索响应格式无效";
            return {};
        }

        std::vector<MciProject> projects;
        for (const auto& hit : response["hits"]) {
            if (!hasString(hit, "project_id") || !hasString(hit, "title")) {
                continue;
            }
            MciProject project;
            project.id = hit["project_id"].get<std::string>();
            project.title = hit["title"].get<std::string>();
            if (hasString(hit, "description")) {
                project.description = hit["description"].get<std::string>();
            }
            if (hasString(hit, "author")) {
                project.author = hit["author"].get<std::string>();
            }
            if (hit.contains("downloads") && hit["downloads"].is_number_integer()) {
                project.downloads = hit["downloads"].get<int>();
            }
            projects.push_back(std::move(project));
        }
        return projects;
    } catch (const Json::exception& error) {
        lastError = "解析 MCI 搜索响应失败: " + std::string(error.what());
        return {};
    }
}

std::vector<MciVersion> MciApiClient::getProjectVersions(
    const std::string& projectId,
    const std::string& gameVersion,
    const std::string& loader) {
    lastError.clear();
    std::string url = "https://mod.mcimirror.top/modrinth/v2/project/" +
        encodePathSegment(projectId) + "/version";
    if (!gameVersion.empty()) {
        url += "?game_versions=" + encodePathSegment(Json::array({gameVersion}).dump());
    }
    if (!loader.empty()) {
        url += (url.find('?') == std::string::npos ? "?" : "&") +
            std::string("loaders=") + encodePathSegment(Json::array({loader}).dump());
    }
    const std::string body = get(url);
    if (body.empty()) {
        return {};
    }

    try {
        const Json response = Json::parse(body);
        if (!response.is_array()) {
            lastError = "MCI 版本响应格式无效";
            return {};
        }

        std::vector<MciVersion> versions;
        for (const auto& item : response) {
            if (!hasString(item, "id")) {
                continue;
            }
            if (!gameVersion.empty() && !containsString(item.value("game_versions", Json::array()), gameVersion)) {
                continue;
            }
            if (!loader.empty() && !containsString(item.value("loaders", Json::array()), loader)) {
                continue;
            }

            MciVersion version;
            version.id = item["id"].get<std::string>();
            if (hasString(item, "version_number")) {
                version.number = item["version_number"].get<std::string>();
            }
            if (item.contains("files") && item["files"].is_array()) {
                for (const auto& file : item["files"]) {
                    if (!hasString(file, "filename")) {
                        continue;
                    }
                    MciVersionFile versionFile;
                    versionFile.fileName = file["filename"].get<std::string>();
                    versionFile.primary = file.value("primary", false);
                    version.files.push_back(std::move(versionFile));
                }
            }
            if (!version.files.empty()) {
                versions.push_back(std::move(version));
            }
        }
        return versions;
    } catch (const Json::exception& error) {
        lastError = "解析 MCI 版本响应失败: " + std::string(error.what());
        return {};
    }
}

std::vector<MciCurseForgeProject> MciApiClient::searchCurseForgeMods(
    const std::string& query,
    const std::string& gameVersion) {
    lastError.clear();
    std::string url = "https://mod.mcimirror.top/curseforge/v1/mods/search?gameId=432&classId=6&pageSize=20&searchFilter=" +
        encodePathSegment(query);
    if (!gameVersion.empty()) {
        url += "&gameVersion=" + encodePathSegment(gameVersion);
    }

    const std::string body = get(url);
    if (body.empty()) {
        return {};
    }

    try {
        const Json response = Json::parse(body);
        if (!response.contains("data") || !response["data"].is_array()) {
            lastError = "MCI CurseForge 搜索响应格式无效";
            return {};
        }

        std::vector<MciCurseForgeProject> projects;
        for (const auto& item : response["data"]) {
            if (!item.contains("id") || !item["id"].is_number_integer() || !hasString(item, "name")) {
                continue;
            }
            MciCurseForgeProject project;
            project.id = std::to_string(item["id"].get<long long>());
            project.title = item["name"].get<std::string>();
            if (hasString(item, "summary")) {
                project.summary = item["summary"].get<std::string>();
            }
            if (item.contains("authors") && item["authors"].is_array() && !item["authors"].empty() &&
                hasString(item["authors"][0], "name")) {
                project.author = item["authors"][0]["name"].get<std::string>();
            }
            if (item.contains("downloadCount") && item["downloadCount"].is_number_integer()) {
                project.downloads = item["downloadCount"].get<long long>();
            }
            projects.push_back(std::move(project));
        }
        return projects;
    } catch (const Json::exception& error) {
        lastError = "解析 MCI CurseForge 搜索响应失败: " + std::string(error.what());
        return {};
    }
}

std::vector<MciCurseForgeFile> MciApiClient::getCurseForgeFiles(
    const std::string& projectId,
    const std::string& gameVersion,
    const std::string& loader) {
    lastError.clear();
    std::string url = "https://mod.mcimirror.top/curseforge/v1/mods/" +
        encodePathSegment(projectId) + "/files?pageSize=50";
    if (!gameVersion.empty()) {
        url += "&gameVersion=" + encodePathSegment(gameVersion);
    }
    if (!loader.empty()) {
        const std::string loaderId = curseForgeLoaderId(loader);
        if (loaderId.empty()) {
            lastError = "不支持的 CurseForge 加载器: " + loader;
            return {};
        }
        url += "&modLoaderType=" + loaderId;
    }

    const std::string body = get(url);
    if (body.empty()) {
        return {};
    }

    try {
        const Json response = Json::parse(body);
        if (!response.contains("data") || !response["data"].is_array()) {
            lastError = "MCI CurseForge 文件响应格式无效";
            return {};
        }

        std::vector<MciCurseForgeFile> files;
        for (const auto& item : response["data"]) {
            if (!item.contains("id") || !item["id"].is_number_integer() || !hasString(item, "fileName")) {
                continue;
            }
            MciCurseForgeFile file;
            file.id = std::to_string(item["id"].get<long long>());
            file.fileName = item["fileName"].get<std::string>();
            if (hasString(item, "displayName")) {
                file.displayName = item["displayName"].get<std::string>();
            }
            if (item.contains("gameVersions") && item["gameVersions"].is_array()) {
                for (const auto& gameVersionItem : item["gameVersions"]) {
                    if (gameVersionItem.is_string()) {
                        file.gameVersions.push_back(gameVersionItem.get<std::string>());
                    }
                }
            }
            files.push_back(std::move(file));
        }
        return files;
    } catch (const Json::exception& error) {
        lastError = "解析 MCI CurseForge 文件响应失败: " + std::string(error.what());
        return {};
    }
}

bool MciApiClient::downloadFile(
    const std::string& projectId,
    const std::string& versionId,
    const std::string& fileName,
    std::string& savedPath) {
    lastError.clear();
    if (fileName.empty() || fileName == "." || fileName == ".." ||
        fileName.find('/') != std::string::npos || fileName.find('\\') != std::string::npos) {
        lastError = "MCI 返回了无效文件名";
        return false;
    }

    const std::string url = "https://mod.mcimirror.top/data/" + encodePathSegment(projectId) +
        "/versions/" + encodePathSegment(versionId) + "/" + encodePathSegment(fileName);
    return downloadUrlToFile(url, fileName, savedPath);
}

bool MciApiClient::downloadCurseForgeFile(
    const std::string& fileId,
    const std::string& fileName,
    std::string& savedPath) {
    lastError.clear();
    if (fileName.empty() || fileName == "." || fileName == ".." ||
        fileName.find('/') != std::string::npos || fileName.find('\\') != std::string::npos) {
        lastError = "MCI 返回了无效文件名";
        return false;
    }

    std::uint64_t numericFileId = 0;
    const auto parseResult = std::from_chars(fileId.data(), fileId.data() + fileId.size(), numericFileId);
    if (parseResult.ec != std::errc{} || parseResult.ptr != fileId.data() + fileId.size()) {
        lastError = "MCI 返回了无效 CurseForge 文件 ID";
        return false;
    }

    std::ostringstream fileIdSuffix;
    fileIdSuffix << std::setw(3) << std::setfill('0') << (numericFileId % 1000);
    const std::string url = "https://mod.mcimirror.top/files/" + std::to_string(numericFileId / 1000) +
        "/" + fileIdSuffix.str() + "/" + encodePathSegment(fileName);
    return downloadUrlToFile(url, fileName, savedPath);
}

bool MciApiClient::downloadUrlToFile(
    const std::string& url,
    const std::string& fileName,
    std::string& savedPath) {
    HINTERNET rawSession = InternetOpenA(
        "ModInjector2/2.0", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!rawSession) {
        lastError = "无法初始化 WinINet: " + std::to_string(GetLastError());
        return false;
    }
    InternetHandle session(rawSession);

    HINTERNET rawRequest = InternetOpenUrlA(
        session.get(), url.c_str(), nullptr, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!rawRequest) {
        lastError = "下载 MCI 文件失败: " + std::to_string(GetLastError());
        return false;
    }
    InternetHandle request(rawRequest);

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    if (!HttpQueryInfoA(request.get(), HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                        &statusCode, &statusSize, nullptr) || statusCode < 200 || statusCode >= 300) {
        lastError = "MCI 文件服务返回 HTTP 状态: " + std::to_string(statusCode);
        return false;
    }

    std::error_code filesystemError;
    const auto downloadDirectory = std::filesystem::current_path() / "downloads" / "MCI";
    std::filesystem::create_directories(downloadDirectory, filesystemError);
    if (filesystemError) {
        lastError = "创建下载目录失败: " + filesystemError.message();
        return false;
    }
    const auto outputPath = downloadDirectory / fileName;
    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output) {
        lastError = "无法创建下载文件: " + outputPath.string();
        return false;
    }

    char buffer[16384];
    DWORD bytesRead = 0;
    while (true) {
        if (!InternetReadFile(request.get(), buffer, sizeof(buffer), &bytesRead)) {
            lastError = "读取 MCI 文件失败: " + std::to_string(GetLastError());
            break;
        }
        if (bytesRead == 0) {
            break;
        }
        output.write(buffer, bytesRead);
        if (!output) {
            lastError = "写入下载文件失败";
            break;
        }
    }
    output.close();
    if (!lastError.empty() || output.fail()) {
        if (lastError.empty()) {
            lastError = "关闭下载文件失败";
        }
        std::filesystem::remove(outputPath, filesystemError);
        return false;
    }

    savedPath = outputPath.string();
    return true;
}

std::string MciApiClient::getLastError() const {
    return lastError;
}

std::string MciApiClient::encodePathSegment(const std::string& value) {
    std::ostringstream encoded;
    encoded << std::uppercase << std::hex;
    for (const unsigned char character : value) {
        if ((character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z') ||
            (character >= '0' && character <= '9') || character == '-' || character == '_' ||
            character == '.' || character == '~') {
            encoded << static_cast<char>(character);
        } else {
            encoded << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(character);
        }
    }
    return encoded.str();
}