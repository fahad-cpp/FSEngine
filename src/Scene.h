#ifndef SCENE_H
#define SCENE_H
#include "Vector.h"
#include <cstdint>
#include <vector>
#define MAX_ENTITIES 32767
struct Transform {
    Vector3 position;
    Vector3 rotation;
    float scale;
};
struct Camera {
    Vector3 position;
    Vector3 rotation;
};
struct Entity {
    uint32_t modelIndex;
    Transform transform;
};
struct Scene {
    Camera camera;
    std::vector<Entity> entities;
};
#endif