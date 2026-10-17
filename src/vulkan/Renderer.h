#ifndef VKRENDERER_H
#define VKRENDERER_H
#include "../Scene.h"
#include "SwapchainContext.h"

struct FrameData {
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAcquireSemaphore;
    VkFence drawFence;
};
struct GraphicsPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};
struct VulkanRenderer {
    GraphicsPipeline pipeline;
    uint32_t frameIndex;
    VkSemaphore renderFinishedSemaphores[MAX_SWAPCHAIN_IMAGES];
    FrameData frames[MAX_FRAMES_IN_FLIGHT];
    std::vector<VulkanEntity> entities; 
    std::vector<VulkanMesh> meshes;
};
VkResult createPipelineLayout(DeviceContext &deviceContext, GraphicsPipeline &pipeline);
VkVertexInputBindingDescription getBindingDescription();
std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription();
VkResult createDescriptorSets(DeviceContext &deviceContext, Entity &entity);

VkShaderModule createShaderModule(VkDevice &device, const std::vector<char> &code);
void createGraphicsPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline, const std::string &shaderPath);
void updateUniformBuffer(const SwapchainContext &swapchainContext,const VulkanRenderer& renderer,const Scene &scene);
void recordCommandBuffer(FrameData frameData, SwapchainContext &swapchainContext,VulkanRenderer &renderer, uint32_t imageIndex, uint32_t frameIndex);
void renderScene(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window, VulkanRenderer &renderer, Scene &scene);
void initPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline);
void cleanupPipeline(DeviceContext &deviceContext, GraphicsPipeline &pipeline);
void setScene(DeviceContext &deviceContext, VulkanRenderer &renderer, Scene &scene);
void initVulkanRenderer(DeviceContext &deviceContext, SwapchainContext &swapchainContext, VulkanRenderer &renderer);
void cleanupVulkanRenderer(DeviceContext &deviceContext, VulkanRenderer &renderer);
#endif