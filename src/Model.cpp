#define _CRT_SECURE_NO_WARNINGS
#include "Model.h"
#include "Timer.h"
#include <cstdio>
#include <fstream>
#include <sstream>

struct OBJIndex {
    uint32_t position;
    uint32_t texture;
    uint32_t normal;
};
OBJModel loadOBJ(const std::string &filename, bool flipYZ) {
    Timer timer;
    startTimer(timer);
    OBJModel mesh;
    std::vector<Vector3> positions = {};
    std::vector<Vector2> texcoords = {};
    std::vector<Vector3> normals = {};
    std::vector<OBJIndex> objIndices = {};

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
    while (*ptr != '\0') {
        const char *end = ptr;
        while ((*end != '\0') && *end != '\n')
            end++;
        line = std::string(ptr, static_cast<std::size_t>(end - ptr));

        if (ptr[0] == 'v' && (ptr[1] == ' ' || ptr[1] == '\t')) {
            float x = 0, y = 0, z = 0;
            if (std::sscanf(line.c_str(), "v %f %f %f", &x, &y, &z) != 3) {
                LOG_ERROR("Unhandled vertex positions");
                return {};
            }
            Vector3 position = {};
            if (flipYZ) {
                position = { x, z, -y };
            } else {
                position = { x, y, z };
            }
            positions.push_back(position);
        } else if (ptr[0] == 'v' && ptr[1] == 't' && (ptr[2] == ' ' || ptr[2] == '\t')) {
            float u = 0.f, v = 0.f, w = 0.f;
            if (std::sscanf(line.c_str(), "vt %f %f %f", &u, &v, &w) != 2) {
                LOG_ERROR("Unhandled textures");
                return {};
            }
            Vector2 tex = { u, 1.f - v };
            texcoords.push_back(tex);
        } else if (ptr[0] == 'v' && ptr[1] == 'n' && (ptr[2] == ' ' || ptr[2] == '\t')) {
            float x = 0.f, y = 0.f, z = 0.f;
            if (std::sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z) != 3) {
                LOG_ERROR("Invalid normals");
                return {};
            }
            Vector3 normal = {};
            if (flipYZ) {
                normal = { x, z, -y };
            } else {
                normal = { x, y, z };
            }
            normals.push_back(normal);
        } else if (ptr[0] == 'f' && (ptr[1] == ' ' || ptr[1] == '\t')) {
            std::istringstream stream(line.c_str() + 1);
            std::vector<OBJIndex> faceIndices;
            faceIndices.reserve(3);
            std::string vertex;
            // Handle arbitrary amount of vertices in a face
            while (stream >> vertex) {
                int v = 0, t = 0, n = 0;
                if (std::sscanf(vertex.c_str(), "%d/%d/%d", &v, &t, &n) == 3) {
                    faceIndices.emplace_back(v - 1, t - 1, n - 1);
                } else if (std::sscanf(vertex.c_str(), "%d//%d", &v, &n) == 2) {
                    faceIndices.emplace_back(v - 1, 0, n - 1);
                } else {
                    LOG_ERROR("Unsupported face format :" << filename);
                    LOG_ERROR("Encountered:" + vertex);
                    return {};
                }
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
    std::vector<OBJIndex> uniqueIndices;
    uint32_t uniqueCount = 0;
    for (uint32_t i = 0; i < objIndices.size(); i++) {
        const OBJIndex index = objIndices[i];
        const auto it = std::find_if(uniqueIndices.begin(), uniqueIndices.end(), [&index](const OBJIndex &objindex) {
            return ((objindex.position == index.position) && (objindex.texture == index.texture) && (objindex.normal == index.normal));
        });
        if (it == uniqueIndices.end()) {
            mesh.vertices.emplace_back(positions[index.position], normals[index.normal], texcoords[index.texture]);
            uniqueIndices.push_back(index);
            mesh.indices.push_back(uniqueCount);
            uniqueCount++;
        } else {
            uint32_t foundIndex = static_cast<uint32_t>(std::distance(uniqueIndices.begin(), it));
            mesh.indices.push_back(foundIndex);
        }
    }
    endTimer(timer);
    LOG_INFO("Succesfully loaded model:" << filename << " : " << timer.diff / 1000.f << " ms");
    return mesh;
}