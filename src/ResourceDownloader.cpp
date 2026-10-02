
#include "ResourceDownloader.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>

ResourceDownloader::ResourceDownloader() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle = curl_easy_init();
}

ResourceDownloader::~ResourceDownloader() {
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
}

size_t ResourceDownloader::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

json ResourceDownloader::MakeApiRequest(const std::string& url) {
    std::string proxied_url = url;
    if (url.find("https://api.github.com/") == 0) {
        proxied_url = "http://localhost:5000/api/" + url.substr(strlen("https://api.github.com/"));
    }

    std::string readBuffer;
    if(curl_handle) {
        curl_easy_reset(curl_handle);
        curl_easy_setopt(curl_handle, CURLOPT_URL, proxied_url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "ResourceDownloader/1.0");
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        CURLcode res = curl_easy_perform(curl_handle);
        if (res != CURLE_OK || readBuffer.empty()) {
            return json{};
        }
    }
    return json::parse(readBuffer);
}

#include <filesystem>
#include <chrono>
#include <iomanip>
#include <iostream>

bool ResourceDownloader::DownloadResource(const std::string& raw_url, const std::string& output_path) {
    if (!curl_handle) return false;

    // Prepare output directory and path
    std::filesystem::path download_dir = std::filesystem::current_path().parent_path() / "MyDownload";
    if (!std::filesystem::exists(download_dir)) {
        std::filesystem::create_directory(download_dir);
    }
    std::filesystem::path full_output_path = download_dir / output_path;

    curl_easy_reset(curl_handle);

    // Use local proxy for GitHub raw content
    std::string proxied_url = raw_url;
    if (raw_url.find("https://github.com/") == 0) {
        // Convert GitHub blob URL to raw URL path
        size_t blob_pos = raw_url.find("/blob/");
        if(blob_pos != std::string::npos) {
            size_t start = raw_url.find("/", 8) + 1;
            std::string repo_path = raw_url.substr(start, blob_pos - start);
            std::string file_path = raw_url.substr(blob_pos + 6);
            proxied_url = "http://localhost:5000/raw/" + repo_path + "/" + file_path;
        }
    } else if (raw_url.find("https://raw.githubusercontent.com/") == 0) {
        // Direct raw URL, replace base with proxy
        std::string base = "https://raw.githubusercontent.com/";
        proxied_url = "http://localhost:5000/raw/" + raw_url.substr(base.length());
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, proxied_url.c_str());

    std::ofstream outFile(full_output_path, std::ios::binary);
    if (!outFile) return false;

    // Set write callback and data
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteToFileCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &outFile);

    // Set user agent and follow redirects
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "ResourceDownloader/1.0");
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    // Disable SSL verification for simplicity
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

    // Set progress callback and enable progress meter
    start_time = std::chrono::steady_clock::now();
    curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
    curl_easy_setopt(curl_handle, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);

    CURLcode res = curl_easy_perform(curl_handle);
    outFile.close();

    if (res != CURLE_OK) {
        std::cerr << "Curl download error: " << curl_easy_strerror(res) << std::endl;
    } else {
        std::cout << std::endl; // Newline after progress bar
    }

    // Disable progress callback after done
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

    return (res == CURLE_OK);
}

int ResourceDownloader::ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    ResourceDownloader* downloader = static_cast<ResourceDownloader*>(clientp);
    if (dltotal == 0) return 0; // Avoid division by zero

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = now - downloader->start_time;
    double speed = dlnow / 1024.0 / elapsed.count(); // KB/s
    double progress = (double)dlnow / dltotal;
    int barWidth = 50;
    int pos = static_cast<int>(barWidth * progress);

    double remaining_time = (dltotal - dlnow) / 1024.0 / speed; // seconds

    std::cout << "\r[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "#";
        else std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(1) << (progress * 100.0) << "% ";
    std::cout << std::fixed << std::setprecision(1) << speed << " KB/s ";
    std::cout << "ETA: " << std::fixed << std::setprecision(1) << remaining_time << "s   " << std::flush;

    return 0;
}

size_t ResourceDownloader::WriteToFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* outFile = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    outFile->write(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::vector<GitHubResource> ResourceDownloader::SearchResources(const std::string& repo, const std::string& filter) {
    std::vector<GitHubResource> result;
    json response = MakeApiRequest("https://api.github.com/repos/" + repo + "/contents/");

    if (response.is_null() || !response.is_array()) {
        return result;
    }

    for(const auto& item : response) {
        if (!item.contains("name") || item["name"].is_null() || !item.contains("download_url") || item["download_url"].is_null()) continue;
        GitHubResource resource;
        resource.name = item["name"].get<std::string>();
        resource.download_url = item["download_url"].get<std::string>();

        if(filter.empty() || resource.name.find(filter) != std::string::npos) {
            result.push_back(resource);
        }
    }
    return result;
}

std::vector<GitHubResource> ResourceDownloader::ListAllResources(const std::string& repo) {
    return SearchResources(repo);
}

std::string ResourceDownloader::GetLatestVersion(const std::string& repo) {
    json response = MakeApiRequest("https://api.github.com/repos/" + repo + "/releases/latest");
    if (response.is_null() || !response.contains("tag_name") || response["tag_name"].is_null()) {
        return "";
    }
    return response["tag_name"].get<std::string>();
}

std::string ResourceDownloader::ConvertToRawUrl(const std::string& github_url) {
    size_t blob_pos = github_url.find("/blob/");
    if(blob_pos != std::string::npos) {
        size_t start = github_url.find("/", 8) + 1;
        std::string repo_path = github_url.substr(start, blob_pos - start);
        std::string file_path = github_url.substr(blob_pos + 6);
        return "https://raw.githubusercontent.com/" + repo_path + "/" + file_path;
    }
    return github_url;
}
