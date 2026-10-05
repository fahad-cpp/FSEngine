#include "AssetManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Timer.h"
#include <filesystem>
#include <fstream>
#include <stb_image.h>
#include <unordered_set>


namespace AssetManager {
std::unordered_set<uint64_t> modelCache;
std::unordered_set<uint64_t> textureCache;
bool                         cacheInitialized = false;
std::hash<std::string>       stringHasher;

static void initCache() {
    if (!std::filesystem::exists("cache")) {
        return;
    }
    modelCache.clear();
    textureCache.clear();
    for (const auto &entry : std::filesystem::directory_iterator("cache")) {
        if (entry.path().extension() != ".fsmodel" && entry.path().extension() != ".fstex") {
            continue;
        }

        const std::string filename = entry.path().stem().string();
        uint64_t          hash     = 0;
        std::from_chars(filename.c_str(), filename.c_str() + filename.length(), hash);
        if (entry.path().extension() == ".fsmodel") {
            modelCache.insert(hash);
        } else if (entry.path().extension() == ".fstex") {
            textureCache.insert(hash);
        }
    }
}
static void cacheModel(const Model &model, uint64_t hash) {
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
static void cacheTexture(uint8_t *texture, const int width, const int height, const int channelSize, const uint64_t hash) {
    const std::string filepath = "cache/" + std::to_string(hash) + ".fstex";
    std::ofstream     ofs(filepath, std::ios::binary);
    ofs.write(reinterpret_cast<const char *>(&width), sizeof(int));
    ofs.write(reinterpret_cast<const char *>(&height), sizeof(int));
    ofs.write(reinterpret_cast<const char *>(&channelSize), sizeof(int));
    ofs.write(reinterpret_cast<const char *>(texture), width * height * 4);
    ofs.close();
}
static uint8_t *loadCachedTexture(uint64_t hash, int *width, int *height, int *channelSize) {
    const std::string filepath = "cache/" + std::to_string(hash) + ".fstex";
    std::ifstream     ifs(filepath, std::ios::binary);
    ifs.read(reinterpret_cast<char *>(width), sizeof(int));
    ifs.read(reinterpret_cast<char *>(height), sizeof(int));
    ifs.read(reinterpret_cast<char *>(channelSize), sizeof(int));
    uint8_t *texture = (uint8_t *)malloc((size_t)(*width * *height * 4));
    ifs.read(reinterpret_cast<char *>(texture), *width * *height * 4);
    ifs.close();
    return texture;
}
Model loadModel(const std::string &filepath, bool flipYZ) {
    Timer timer;
    startTimer(timer);
    if (!cacheInitialized) {
        initCache();
        cacheInitialized = true;
    }

    uint64_t hash = stringHasher(filepath);
    if (modelCache.find(hash) != modelCache.end()) {
        Model model = loadCachedModel(hash);
        endTimer(timer);
        LOG_INFO("Loaded cached model " << hash << ".fsmodel" << " : " << microsecToms(timer.diff) << " ms");
        return model;
    }

    Model model = loadOBJ(filepath, flipYZ);
    cacheModel(model, hash);
    endTimer(timer);
    LOG_INFO("Loaded new model " << filepath << " : " << microsecToms(timer.diff) << " ms");
    return model;
}
uint8_t *loadTexture(const std::string &filepath, int *texWidth, int *texHeight, int *channelSize) {
    Timer timer;
    startTimer(timer);
    if (!cacheInitialized) {
        initCache();
        cacheInitialized = true;
    }
    uint64_t hash = stringHasher(filepath);
    if (textureCache.find(hash) != textureCache.end()) {
        uint8_t *texture = loadCachedTexture(hash, texWidth, texHeight, channelSize);
        endTimer(timer);
        LOG_INFO("Loaded cached texture: " << hash << ".fstex" << " : " << microsecToms(timer.diff) << " ms");
        return texture;
    }

    uint8_t *texture = stbi_load(filepath.c_str(), texWidth, texHeight, channelSize, STBI_rgb_alpha);
    cacheTexture(texture, *texWidth, *texHeight, *channelSize, hash);
    endTimer(timer);
    LOG_INFO("Loaded new texture: " << filepath << " : " << microsecToms(timer.diff) << " ms");
    return texture;
}
void unloadTexture(uint8_t *texture) {
    free(texture);
}
} // namespace AssetManager