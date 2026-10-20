#ifndef SCENE_H
#define SCENE_H
#include "Model.h"
#include <vector>
#define MAX_ENTITIES 15625
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
    ModelHandle modelHandle;
    Transform transform;
};
struct Scene {
    Camera camera;
    std::vector<Entity> entities;
};
#endif