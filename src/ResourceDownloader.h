
#ifndef RESOURCE_DOWNLOADER_H
#define RESOURCE_DOWNLOADER_H



#pragma once
#include <string>
#include <vector>
#include <curl/curl.h>
#include <curl/easy.h>
#include "json.hpp"

using json = nlohmann::json;

struct GitHubResource {
    std::string name;
    std::string download_url;
    std::string version;
    std::string description;
};

#include <chrono>
#include <iostream>
#include <iomanip>

class ResourceDownloader {
public:
    ResourceDownloader();
    ~ResourceDownloader();

    bool DownloadResource(const std::string& raw_url, const std::string& output_path);
    std::vector<GitHubResource> SearchResources(const std::string& repo, const std::string& filter = "");
    std::vector<GitHubResource> ListAllResources(const std::string& repo);
    std::string GetLatestVersion(const std::string& repo);

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t WriteToFileCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
    json MakeApiRequest(const std::string& url);
    std::string ConvertToRawUrl(const std::string& github_url);
    CURL* curl_handle;

    std::chrono::steady_clock::time_point start_time;
};


#endif //RESOURCE_DOWNLOADER_H