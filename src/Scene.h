#ifndef SCENE_H
#define SCENE_H
#include "Vector.h"
#include "vulkan/Resource.h"
#define MAX_ENTITIES 256
struct Camera{
    Vector3 position;
    Vector3 rotation;
};
struct Entity{
    Mesh mesh;
    Vector3 position;
    Vector3 rotation;
    float scale;
    Buffer uniformBuffer;
    void* uniformBufferMapping;
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
};
struct Scene{
    Camera camera;
    std::vector<Entity> entities;
};

void initScene(DeviceContext& deviceContext,Scene& scene);
void cleanupScene(DeviceContext& deviceContext,Scene& scene);
#endif