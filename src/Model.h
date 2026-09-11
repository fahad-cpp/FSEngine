#ifndef MODEL_H
#define MODEL_H
#include "Vector.h"
#include <vector>
#include <string>
#include <cstdint>
struct Vertex {
    Vector3 pos;
    Vector3 color;
    Vector2 texCoord;
};

struct OBJModel{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

OBJModel loadOBJ(const std::string& filepath,bool flipYZ=false);
#endif