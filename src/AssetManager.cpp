#include "AssetManager.h"
#include "Timer.h"
#include <filesystem>
#include <fstream>
#include <unordered_set>

namespace AssetManager {
std::unordered_set<uint64_t> modelCache;
bool                         cacheInitialized = false;

static void initCache() {
    if (!std::filesystem::exists("cache")) {
        return;
    }
    for (const auto &entry : std::filesystem::directory_iterator("cache")) {
        if (entry.path().extension() != ".fsmodel") {
            continue;
        }

        const std::string filename = entry.path().stem().string();
        uint64_t          hash     = 0;
        std::from_chars(filename.c_str(), filename.c_str() + filename.length(), hash);

        modelCache.insert(hash);
    }
}
static void exportModel(const Model &model, uint64_t hash) {
    std::filesystem::create_directory("cache");
    const std::string filepath = "cache/" + std::to_string(hash) + ".fsmodel";
    std::ofstream     ofs(filepath, std::ios::binary);

    uint32_t vertexCount = static_cast<uint32_t>(model.vertices.size());
    ofs.write(reinterpret_cast<const char *>(&vertexCount), sizeof(uint32_t));
    ofs.write(reinterpret_cast<const char *>(model.vertices.data()), static_cast<std::streamsize>(model.vertices.size() * sizeof(Vertex)));

    uint32_t indexCount = static_cast<uint32_t>(model.indices.size());
    ofs.write(reinterpret_cast<const char *>(&indexCount), sizeof(uint32_t));
    ofs.write(reinterpret_cast<const char *>(model.indices.data()), static_cast<std::streamsize>(model.indices.size() * sizeof(uint32_t)));

    ofs.close();
}
static Model loadCachedModel(uint64_t hash) {
    std::string   filepath = "cache/" + std::to_string(hash) + ".fsmodel";
    std::ifstream ifs(filepath, std::ios::binary);
    uint32_t      verticesCount = 0;
    uint32_t      indicesCount  = 0;

    ifs.read(reinterpret_cast<char *>(&verticesCount), sizeof(uint32_t));
    std::vector<Vertex> vertices(verticesCount);
    ifs.read(reinterpret_cast<char *>(vertices.data()), static_cast<std::streamsize>(verticesCount * sizeof(Vertex)));

    ifs.read(reinterpret_cast<char *>(&indicesCount), sizeof(uint32_t));
    std::vector<uint32_t> indices(indicesCount);
    ifs.read(reinterpret_cast<char *>(indices.data()), static_cast<std::streamsize>(indicesCount * sizeof(uint32_t)));

    ifs.close();
    Model model = {
        vertices,
        indices
    };
    return model;
}
Model loadModel(const std::string &filepath, bool flipYZ) {
    Timer timer;
    startTimer(timer);
    if (!cacheInitialized) {
        initCache();
        cacheInitialized = true;
    }
    std::hash<std::string> hasher;

    uint64_t hash = hasher(filepath);
    if (modelCache.find(hash) != modelCache.end()) {
        endTimer(timer);
        LOG_INFO("Loaded cached model " << hash << ".fsmodel" << " : " << microsecToms(timer.diff) << " ms");
        return loadCachedModel(hash);
    }

    Model model = loadOBJ(filepath, flipYZ);
    exportModel(model, hash);
    endTimer(timer);
    LOG_INFO("Loaded new model " << filepath << " : " << microsecToms(timer.diff) << " ms");
    return model;
}
} // namespace AssetManager