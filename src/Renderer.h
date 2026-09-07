#ifndef RENDERER_H
#define RENDERER_H
#include "SwapchainContext.h"
#include "Vector.h"
#include <array>
#include <string>
#include <vector>
#include "Matrix.h"

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct Buffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
};
struct Image{
    VkImage image;
    VkDeviceMemory imageMemory;
};
struct UniformBufferData {
    alignas(16) Matrix4 model;
    alignas(16) Matrix4 view;
    alignas(16) Matrix4 projection;
};
struct Mesh {
    Buffer vertexBuffer;
    Buffer indexBuffer;
    uint32_t vertexCount;
    uint32_t indexCount;
};
struct Vertex {
    Vector3 pos;
    Vector3 color;
};
struct FrameData {
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAcquireSemaphore;
    VkFence drawFence;
    Buffer uniformBuffer;
    void *uniformBufferMapping;
};
struct GraphicsPipeline {
    VkPipeline pipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
};
struct Renderer {
    uint32_t frameIndex;
    VkSemaphore renderFinishedSemaphores[MAX_SWAPCHAIN_IMAGES];
    FrameData frames[MAX_FRAMES_IN_FLIGHT];
    GraphicsPipeline pipeline;
};

VkShaderModule createShaderModule(VkDevice &device, const std::vector<char> &code);
VkResult createDescriptorPool(DeviceContext &deviceContext, GraphicsPipeline &pipeline);
VkResult createDescriptorSets(DeviceContext& deviceContext, GraphicsPipeline& pipeline, FrameData* frames);
VkResult createDescriptorSetLayout(DeviceContext &deviceContext);
VkResult createPipelineLayout(DeviceContext &deviceContext, GraphicsPipeline &pipeline);
VkVertexInputBindingDescription getBindingDescription();
std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription();

void createGraphicsPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline, const std::string &shaderPath);
Buffer createBuffer(DeviceContext &deviceContext, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memoryProperty);
Image createImage(DeviceContext& deviceContext,uint32_t width,uint32_t height,VkFormat format,VkImageTiling tiling,VkImageUsageFlags usage,VkMemoryPropertyFlags memoryFlags);
void createTextureImage(DeviceContext& deviceContext,const std::string& filepath,Image& textureImage);
void createVertexBuffer(DeviceContext &deviceContext, const Vertex *vertices, uint32_t vertexCount, Buffer &vertexBuffer);
void createIndexBuffer(DeviceContext &deviceContext, const uint32_t *indices, uint32_t indexCount, Buffer &indexBuffer);
void createUniformBuffer(DeviceContext &deviceContext, const UniformBufferData uniformBufferData, Buffer &uniformBuffer, void **pMapped);
void updateUniformBuffer(FrameData &frame, SwapchainContext &swapchainContext);
void recordCommandBuffer(FrameData frameData, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline, Mesh &mesh, uint32_t imageIndex);
void drawFrame(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window, Renderer &renderer, Mesh &mesh);
void initPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline);
void cleanupPipeline(DeviceContext &deviceContext, GraphicsPipeline &pipeline);
void initMesh(DeviceContext &deviceContext, Mesh &mesh, const Vertex *vertices, uint32_t vertexCount, const uint32_t *indices, uint32_t indexCount);
void cleanupMesh(DeviceContext &deviceContext, Mesh &mesh);
void initRenderer(DeviceContext &deviceContext, SwapchainContext &swapchainContext, Renderer &renderer);
void cleanupRenderer(DeviceContext &deviceContext, Renderer &renderer);
#endif