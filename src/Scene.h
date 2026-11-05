#ifndef SCENE_H
#define SCENE_H
#include "Model.h"
#include <vector>
#define MAX_ENTITIES 97336
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
enum class LightType : uint8_t{
    LT_DIRECTIONAL,
    LT_POINT
};
struct Light{
    Vector3 vec;
    float radius;
    float illum;
    LightType type;
};
struct Scene {
    std::vector<Entity> entities;
    std::vector<Light> lights;
};
#endif