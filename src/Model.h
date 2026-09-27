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

struct Model {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
};

Model loadOBJ(const std::string &filepath, bool flipYZ = false);
#endif