#ifndef MODEL_H
#define MODEL_H
#include "Renderer.h"

struct OBJModel{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

OBJModel loadOBJ(const std::string& filepath);
#endif