#include "Utilities.h"
#include <fstream>
#include <iostream>

const std::vector<char> readFile(const std::string &path) {
    std::ifstream ifs(path, std::ios::ate | std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file:" << path << "\n";
        return {};
    }
    std::size_t fileSize = static_cast<std::size_t>(ifs.tellg());
    ifs.seekg(0);
    std::vector<char> fileContent(fileSize);
    ifs.read(fileContent.data(), static_cast<std::streamsize>(fileSize));
    ifs.close();
    return fileContent;
}

