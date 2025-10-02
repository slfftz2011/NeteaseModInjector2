#include "../JsonParser.h"
#include <iostream>

int main() {
    JsonParser parser;

    std::string testJsonPath = "components/test/list.json";

    std::cout << "Testing JsonParser loadListJson with: " << testJsonPath << std::endl;
    bool loadResult = parser.loadListJson(testJsonPath);
    std::cout << "loadListJson result: " << (loadResult ? "Success" : "Failure") << std::endl;
    if (!loadResult) {
        std::cerr << "Error: " << parser.getLastError() << std::endl;
    }

    // Test verifyFile with a sample file path relative to components root
    std::string testFile = "mods/example_mod.jar";
    std::cout << "Testing verifyFile with: " << testFile << std::endl;
    bool verifyResult = parser.verifyFile(testFile);
    std::cout << "verifyFile result: " << (verifyResult ? "Success" : "Failure") << std::endl;
    if (!verifyResult) {
        std::cerr << "Error: " << parser.getLastError() << std::endl;
    }

    std::cout << "JsonParser tests completed." << std::endl;
    return 0;
}
