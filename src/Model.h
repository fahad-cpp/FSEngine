#ifndef MODEL_H
#define MODEL_H
#include "Vector.h"
#include <cstdint>
#include <string>
#include <vector>
struct Vertex {
    Vector3 pos;
    Vector3 normal;
    Vector2 texCoord;
};

struct OBJIndex {
    uint32_t position;
    uint32_t texture;
    uint32_t normal;
};

struct OBJModel {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
};

OBJModel loadOBJ(const std::string &filepath, bool flipYZ = false);
#endif