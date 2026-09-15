#ifndef RENDERER_H
#define RENDERER_H
#include "../Scene.h"
#include "SwapchainContext.h"

struct FrameData {
    VkCommandBuffer commandBuffer;
    VkSemaphore     imageAcquireSemaphore;
    VkFence         drawFence;
};
struct GraphicsPipeline {
    VkPipeline       pipeline;
    VkPipelineLayout pipelineLayout;
};
struct VulkanRenderer {
    GraphicsPipeline pipeline;
    uint32_t         frameIndex;
    VkSemaphore      renderFinishedSemaphores[MAX_SWAPCHAIN_IMAGES];
    FrameData        frames[MAX_FRAMES_IN_FLIGHT];
};
VkResult                                         createPipelineLayout(DeviceContext &deviceContext, GraphicsPipeline &pipeline);
VkVertexInputBindingDescription                  getBindingDescription();
std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription();

VkShaderModule createShaderModule(VkDevice &device, const std::vector<char> &code);
void           createGraphicsPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline, const std::string &shaderPath);
void           updateUniformBuffer(SwapchainContext &swapchainContext, Scene &scene);
void           recordCommandBuffer(FrameData frameData, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline, Scene &scene, uint32_t imageIndex, uint32_t frameIndex);
void           renderScene(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window, VulkanRenderer &renderer, Scene &scene);
void           initPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline);
void           cleanupPipeline(DeviceContext &deviceContext, GraphicsPipeline &pipeline);
void           initVulkanRenderer(DeviceContext &deviceContext, SwapchainContext &swapchainContext, VulkanRenderer &renderer);
void           cleanupVulkanRenderer(DeviceContext &deviceContext, VulkanRenderer &renderer);
#endif