#include "JsonParser.h"
#include <fstream>

bool JsonParser::loadListJson(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        lastError = "文件不存在: " + filePath;
        return false;
    }
    try {
        file >> jsonData;
    } catch (const nlohmann::json::parse_error& e) {
        lastError = "JSON解析错误: " + std::string(e.what());
        return false;
    }
    // Validate JSON structure
    if (!jsonData.is_object()) {
        lastError = "JSON根元素不是对象";
        return false;
    }
    if (!jsonData.contains("files") || !jsonData["files"].is_array()) {
        lastError = "JSON缺少有效的'files'数组";
        return false;
    }
    // Validate each file entry
    for (const auto& item : jsonData["files"]) {
        if (!item.is_object() || !item.contains("path") || !item["path"].is_string()) {
            lastError = "JSON中'files'数组的条目无效";
            return false;
        }
    }
    return true;
}

bool JsonParser::verifyFile(const std::string& relativePath) {
    if (!jsonData.contains("files") || !jsonData["files"].is_array()) {
        lastError = "JSON数据无效，无法验证文件";
        return false;
    }
    for (const auto& item : jsonData["files"]) {
        if (item.contains("path") && item["path"].is_string() && item["path"] == relativePath) {
            return true;
        }
    }
    lastError = "文件未在list.json中列出: " + relativePath;
    return false;
}

std::string JsonParser::getLastError() const {
    return lastError;
}