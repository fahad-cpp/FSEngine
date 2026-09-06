#define _CRT_SECURE_NO_WARNINGS
#include "Model.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>
#include <cstdio>
OBJModel loadOBJ(const std::string& filename){
    std::vector<Vertex> vertices = {};
    std::vector<uint32_t> indices = {};
    std::random_device rd;
    std::mt19937 engine(static_cast<std::mt19937>(rd()));
    std::uniform_real_distribution<float> dist(0.f,1.f);
    

    std::ifstream OBJFile(filename, std::ios::binary | std::ios::ate);
    if (!OBJFile) {
        std::cerr << "Cannot open file " << filename << "\n";
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
            std::sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
            Vector3 vertex = {x,y,z};
            Vector3 color = {dist(engine),dist(engine),dist(engine)};
            color.x *= color.x;
            color.y *= color.y;
            color.z *= color.z;
            vertices.emplace_back(vertex,color);
        } else if (ptr[0] == 'v' && ptr[1] == 't' && (ptr[2] == ' ' || ptr[2] == '\t')) {
            // handle textures
        } else if (ptr[0] == 'v' && ptr[1] == 'n' && (ptr[2] == ' ' || ptr[2] == '\t')) {
            //handle normals
        } else if (ptr[0] == 'f' && (ptr[1] == ' ' || ptr[1] == '\t')) {
            std::istringstream stream(line.c_str() + 1);
            std::vector<uint32_t> faceIndices;
            faceIndices.reserve(3);
            std::string vertex;
            // Handle arbitrary amount of vertices in a face
            while (stream >> vertex) {
                int v, t, n;
                if (std::sscanf(vertex.c_str(), "%d/%d/%d", &v, &t, &n) == 3) {
                    faceIndices.emplace_back(v - 1);
                } else if (std::sscanf(vertex.c_str(), "%d//%d", &v, &n) == 2) {
                    faceIndices.emplace_back(v - 1);
                } else {
                    std::cerr << "Unsupported face format :" << filename << "\n";
                    std::cerr << "Encountered:" + vertex;
                    return {};
                }
            }
            if (faceIndices.size() < 3) {
                std::cerr << "Less than 3 points in face: " << filename << "\n";
                return {};
            }

            // Use TRIANGLE_FAN ordering
            for (std::size_t i = 2; i < faceIndices.size(); i++) {
                indices.emplace_back(faceIndices[0]);
                indices.emplace_back(faceIndices[i-1]);
                indices.emplace_back(faceIndices[i]);
            }
        }

        //skip until EOF or newline
        while ((*ptr != '\0') && *ptr != '\n')
            ptr++;
        //skip newline
        if (*ptr == '\n')
            ptr++;
    }
    OBJModel mesh = {
        vertices,
        indices
    };
    std::cout << "Succesfully loaded model:" << filename << "\n";
    return mesh;
}