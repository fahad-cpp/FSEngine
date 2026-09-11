#ifndef RENDERER_H
#define RENDERER_H
#include "Resource.h"
#include "SwapchainContext.h"


constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
struct FrameData {
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAcquireSemaphore;
    VkFence drawFence;
    Buffer uniformBuffer;
    void *uniformBufferMapping;
};
struct GraphicsPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
};
struct VulkanRenderer {
    GraphicsPipeline pipeline;
    uint32_t frameIndex;
    VkSemaphore renderFinishedSemaphores[MAX_SWAPCHAIN_IMAGES];
    FrameData frames[MAX_FRAMES_IN_FLIGHT];
};

VkResult createDescriptorPool(DeviceContext &deviceContext, GraphicsPipeline &pipeline);
VkResult createDescriptorSets(DeviceContext &deviceContext, GraphicsPipeline &pipeline, FrameData *frames, Mesh &mesh);
VkResult createDescriptorSetLayout(DeviceContext &deviceContext);
VkResult createPipelineLayout(DeviceContext &deviceContext, GraphicsPipeline &pipeline);
VkVertexInputBindingDescription getBindingDescription();
std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription();

VkShaderModule createShaderModule(VkDevice &device, const std::vector<char> &code);
void createGraphicsPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline, const std::string &shaderPath);

void updateUniformBuffer(FrameData &frame, SwapchainContext &swapchainContext);
void recordCommandBuffer(FrameData frameData, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline, Mesh &mesh, uint32_t imageIndex, uint32_t frameIndex);
void drawFrame(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window, VulkanRenderer &renderer, Mesh &mesh);
void initPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline);
void cleanupPipeline(DeviceContext &deviceContext, GraphicsPipeline &pipeline);
void initVulkanRenderer(DeviceContext &deviceContext, SwapchainContext &swapchainContext, VulkanRenderer &renderer, Mesh &mesh);
void cleanupVulkanRenderer(DeviceContext &deviceContext, VulkanRenderer &renderer);
#endif