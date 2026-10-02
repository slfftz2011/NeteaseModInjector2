
#ifndef MODINJECTOR_JSONPARSER_H
#define MODINJECTOR_JSONPARSER_H


#pragma once
#include <string>
#include <filesystem>
#include "json.hpp"

namespace fs = std::filesystem;

class JsonParser {
public:
    JsonParser() = default;
    bool loadListJson(const std::string& filePath);
    bool verifyFile(const std::string& relativePath);
    std::string getLastError() const;

private:
    std::string lastError;
    nlohmann::json jsonData;
};

#endif // MODINJECTOR_JSONPARSER_H