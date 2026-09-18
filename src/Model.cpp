#define _CRT_SECURE_NO_WARNINGS
#include "Model.h"
#include "Timer.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <unordered_map>
namespace std {
template <>
struct hash<OBJIndex> {
    size_t operator()(OBJIndex const &index) const {
        uint64_t hash         = index.position;
        uint64_t hashConstant = 0x9E3779B185EBCA87ULL;
        hash                  = hash * hashConstant + index.texture;
        hash                  = hash * hashConstant + index.normal;
        return hash;
    }
};
}; // namespace std
bool operator==(const OBJIndex &ind1, const OBJIndex &ind2) {
    return ((ind1.position == ind2.position) && (ind1.texture == ind2.texture) && (ind1.normal == ind2.normal));
}
inline static bool isNumeric(char c) {
    return (c >= '0' && c <= '9');
}
inline static char *getfloat(char *ptr, float *value) {
    while (!isNumeric(*ptr) && (*ptr != '-')) {
        ptr++;
    }
    char *end = ptr;
    while (isNumeric(*end) || (*end == '.') || (*end == '-')) {
        end++;
    }
    std::from_chars(ptr, end, *value);
    return end;
}
inline static char *getuint(char *ptr, uint32_t *value) {
    while (!isNumeric(*ptr)) {
        ptr++;
    }
    char *end = ptr;
    while (isNumeric(*end)) {
        end++;
    }
    std::from_chars(ptr, end, *value);
    return end;
}
// get floats seperated by space
inline static Vector3 get3floats(char *ptr) {
    Vector3 res;
    ptr = getfloat(ptr, &res.x);
    ptr = getfloat(ptr + 1, &res.y);
    ptr = getfloat(ptr + 1, &res.z);
    return res;
}
inline static Vector2 get2floats(char *ptr) {
    Vector2 res;
    ptr = getfloat(ptr, &res.x);
    ptr = getfloat(ptr + 1, &res.y);
    return res;
}

inline static void getIndices(char *ptr, uint32_t &v, uint32_t &t, uint32_t &n) {
    while (!isNumeric(*ptr)) {
        ptr++;
    }
    ptr = getuint(ptr, &v);
    if (*ptr != '/') {
        return;
    }
    ptr++;
    if (*ptr != '/') {
        ptr = getuint(ptr, &t);
    }

    if (*ptr != '/') {
        return;
    }
    ptr++;
    ptr = getuint(ptr, &n);
}
OBJModel loadOBJ(const std::string &filename, bool flipYZ) {
    Timer timer;
    startTimer(timer);
    OBJModel      mesh;
    std::ifstream OBJFile(filename, std::ios::binary | std::ios::ate);
    if (!OBJFile) {
        LOG_ERROR("Cannot open file " << filename);
        return {};
    }

    std::size_t size = static_cast<std::size_t>(OBJFile.tellg());
    OBJFile.seekg(0);

    std::vector<char> buffer(size + 1);
    OBJFile.read(buffer.data(), static_cast<std::streamsize>(size));
    buffer[size] = '\0';
    OBJFile.close();

    const char *ptr = buffer.data();
    std::string line;

    uint32_t positionsCount = 0;
    uint32_t normalsCount   = 0;
    uint32_t texCoordCount  = 0;
    uint32_t indicesCount   = 0;
    while (*ptr != '\0') {
        if (ptr[0] == 'v') {
            if (ptr[1] == ' ' || ptr[1] == '\t') {
                ++positionsCount;
            } else if (ptr[1] == 't' && (ptr[2] == ' ' || ptr[2] == '\t')) {
                ++texCoordCount;
            } else if (ptr[1] == 'n' && (ptr[2] == ' ' || ptr[2] == '\t')) {
                ++normalsCount;
            }
        } else if (ptr[0] == 'f') {
            uint32_t faceVerticesCount = 0;
            while (*ptr != '\n') {
                if (*ptr == ' ' || *ptr == '\t') {
                    ++faceVerticesCount;
                    // skip whitespace between face vertices
                    while (*ptr == ' ' || *ptr == '\t') {
                        ptr++;
                    }
                }
                ptr++;
            }
            indicesCount += (faceVerticesCount - 2) * 3;
        } else {
            ptr++;
            continue;
        }
        while ((*ptr != '\0') && *ptr != '\n')
            ptr++;
        if (*ptr == '\n')
            ptr++;
    }
    ptr = buffer.data();
    std::vector<Vector3>  positions(positionsCount);
    std::vector<Vector3>  normals(normalsCount);
    std::vector<Vector2>  texcoords(texCoordCount);
    std::vector<OBJIndex> objIndices;
    objIndices.reserve(indicesCount);
    uint32_t vi = 0, ni = 0, ti = 0;
    while (*ptr != '\0') {
        const char *end = ptr;
        while ((*end != '\0') && *end != '\n') {
            end++;
        }
        line = std::string(ptr, static_cast<std::size_t>(end - ptr));

        if (ptr[0] == 'v' && (ptr[1] == ' ' || ptr[1] == '\t')) {
            Vector3 vec      = get3floats(line.data() + 2);
            Vector3 position = {};
            if (flipYZ) {
                position = { vec.x, vec.z, -vec.y };
            } else {
                position = { vec.x, vec.y, vec.z };
            }
            positions[vi++] = position;
        } else if (ptr[0] == 'v' && ptr[1] == 't' && (ptr[2] == ' ' || ptr[2] == '\t')) {
            Vector2 vec     = get2floats(line.data() + 3);
            Vector2 tex     = { vec.x, 1.f - vec.y };
            texcoords[ti++] = tex;
        } else if (ptr[0] == 'v' && ptr[1] == 'n' && (ptr[2] == ' ' || ptr[2] == '\t')) {
            Vector3 vec    = get3floats(line.data() + 3);
            Vector3 normal = {};
            if (flipYZ) {
                normal = { vec.x, vec.z, -vec.y };
            } else {
                normal = { vec.x, vec.y, vec.z };
            }
            normals[ni++] = normal;
        } else if (ptr[0] == 'f' && (ptr[1] == ' ' || ptr[1] == '\t')) {
            std::istringstream    stream(line.c_str() + 2);
            std::vector<OBJIndex> faceIndices;
            faceIndices.reserve(3);
            std::string vertex;
            // Handle arbitrary amount of vertices in a face
            while (stream >> vertex) {
                uint32_t v = 0, t = 0, n = 0;
                getIndices(vertex.data(), v, t, n);
                faceIndices.emplace_back(v - 1, t - 1, n - 1);
            }
            if (faceIndices.size() < 3) {
                LOG_ERROR("Less than 3 points in face: " << filename);
                return {};
            }

            // Use TRIANGLE_FAN ordering
            for (std::size_t i = 2; i < faceIndices.size(); i++) {
                objIndices.emplace_back(faceIndices[0]);
                objIndices.emplace_back(faceIndices[i - 1]);
                objIndices.emplace_back(faceIndices[i]);
            }
        }

        // skip until EOF or newline
        while ((*ptr != '\0') && *ptr != '\n')
            ptr++;
        // skip newline
        if (*ptr == '\n')
            ptr++;
    }

    // structure obj into unique vertices and indices
    std::unordered_map<OBJIndex, uint32_t> uniqueIndices;

    uint32_t uniqueCount = 0;
    mesh.indices.reserve(indicesCount);
    mesh.vertices.reserve(positionsCount);
    for (uint32_t i = 0; i < objIndices.size(); i++) {
        const OBJIndex index = objIndices[i];
        const auto     it    = uniqueIndices.find(index);
        if (it == uniqueIndices.end()) {
            Vector3 normal   = { 0.f, 0.f, 0.f };
            Vector3 position = { 0.f, 0.f, 0.f };
            Vector2 texcoord = { 0.f, 0.f };
            if (positions.size()) {
                position = positions[index.position];
            }
            if (texcoords.size()) {
                texcoord = texcoords[index.texture];
            }
            if (normals.size()) {
                normal = normals[index.normal];
            }
            mesh.vertices.emplace_back(position, normal, texcoord);
            mesh.indices.emplace_back(uniqueCount);
            uniqueIndices[index] = uniqueCount;
            uniqueCount++;
        } else {
            uint32_t foundIndex = uniqueIndices[index];
            mesh.indices.emplace_back(foundIndex);
        }
    }
    endTimer(timer);
    LOG_INFO("Succesfully loaded model:" << filename << " : " << timer.diff / 1000.f << " ms");
    return mesh;
}