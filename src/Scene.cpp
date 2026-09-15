#include "Scene.h"
void initScene(DeviceContext& deviceContext,Scene& scene){
    for(Entity& entity : scene.entities){
        entity.uniformBuffer = createUniformBuffer(deviceContext, &entity.uniformBufferMapping);
        createDescriptorSets(deviceContext, entity);
    }
}
void cleanupScene(DeviceContext& deviceContext,Scene& scene){
    for(Entity& entity : scene.entities){
        vkUnmapMemory(deviceContext.device, entity.uniformBuffer.memory);
        entity.uniformBufferMapping = nullptr;
        cleanupBuffer(deviceContext, entity.uniformBuffer);
        vkFreeDescriptorSets(deviceContext.device, deviceContext.descriptorPool, MAX_FRAMES_IN_FLIGHT, entity.descriptorSets);
    }
}