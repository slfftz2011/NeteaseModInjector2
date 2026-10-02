
#include "../ResourceDownloader.h"
#include <iostream>

int main() {
    ResourceDownloader downloader;

    // 示例：搜索资源
    auto resources = downloader.SearchResources("slfftz2011/NeteaseModInjector2", "components/test.COP");
    for(const auto& res : resources) {
        std::cout << "Found Resource: " << res.name << " (" << res.download_url << ")" << std::endl;
    }

    // 示例：下载文件，使用本地代理地址
    if(downloader.DownloadResource(
        "https://github.com/slfftz2011/NeteaseModInjector2/blob/refs/tags/v2.0.0-rc1/components/test.COP",
        "tacz.COP")) {
        std::cout << "File download successful!" << std::endl;
    }

    return 0;
}
