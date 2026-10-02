#ifndef MODINJECTOR2_MCIAPICLIENT_H
#define MODINJECTOR2_MCIAPICLIENT_H

#include <string>
#include <vector>

struct MciProject {
    std::string id;
    std::string title;
    std::string description;
    std::string author;
    int downloads = 0;
};

struct MciVersionFile {
    std::string fileName;
    bool primary = false;
};

struct MciVersion {
    std::string id;
    std::string number;
    std::vector<MciVersionFile> files;
};

struct MciCurseForgeProject {
    std::string id;
    std::string title;
    std::string summary;
    std::string author;
    long long downloads = 0;
};

struct MciCurseForgeFile {
    std::string id;
    std::string fileName;
    std::string displayName;
    std::vector<std::string> gameVersions;
};

class MciApiClient {
public:
    std::vector<MciProject> searchMods(
        const std::string& query,
        const std::string& gameVersion,
        const std::string& loader);
    std::vector<MciVersion> getProjectVersions(
        const std::string& projectId,
        const std::string& gameVersion,
        const std::string& loader);
    std::vector<MciCurseForgeProject> searchCurseForgeMods(
        const std::string& query,
        const std::string& gameVersion);
    std::vector<MciCurseForgeFile> getCurseForgeFiles(
        const std::string& projectId,
        const std::string& gameVersion,
        const std::string& loader);
    bool downloadFile(
        const std::string& projectId,
        const std::string& versionId,
        const std::string& fileName,
        std::string& savedPath);
    bool downloadCurseForgeFile(
        const std::string& fileId,
        const std::string& fileName,
        std::string& savedPath);
    std::string getLastError() const;

private:
    std::string lastError;

    std::string get(const std::string& url);
    bool downloadUrlToFile(
        const std::string& url,
        const std::string& fileName,
        std::string& savedPath);
    static std::string encodePathSegment(const std::string& value);
};

#endif