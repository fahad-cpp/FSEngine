#include "SwapchainContext.h"
#include "Vector.h"
#include <array>
#include <string>
#include <vector>

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct Buffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
};
struct Mesh {
    Buffer vertexBuffer;
    Buffer indexBuffer;
    uint32_t vertexCount;
    uint32_t indexCount;
};
struct Vertex {
    Vector2 pos;
    Vector3 color;
};
struct FrameData {
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAcquireSemaphore;
    VkFence drawFence;
};
struct VulkanPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};
struct Renderer {
    uint32_t frameIndex;
    VkSemaphore renderFinishedSemaphores[MAX_SWAPCHAIN_IMAGES];
    FrameData frames[MAX_FRAMES_IN_FLIGHT];
    VulkanPipeline pipeline;
};

VkShaderModule createShaderModule(VkDevice &device, const std::vector<char> &code);
VkResult createPipelineLayout(DeviceContext &deviceContext, VulkanPipeline &pipeline);
VkVertexInputBindingDescription getBindingDescription();
std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription();

void createGraphicsPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, VulkanPipeline &pipeline, const std::string &shaderPath);
Buffer createBuffer(DeviceContext &deviceContext, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memoryProperty);
void createVertexBuffer(DeviceContext &deviceContext, const Vertex *vertices, uint32_t vertexCount, Buffer &vertexBuffer);
void createIndexBuffer(DeviceContext &deviceContext, const uint32_t *indices, uint32_t indexCount, Buffer &indexBuffer);
void recordCommandBuffer(FrameData frameData, SwapchainContext &swapchainContext, VulkanPipeline &pipeline, Mesh &mesh, uint32_t imageIndex);
void drawFrame(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window, Renderer &renderer, Mesh &mesh);
void initPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, VulkanPipeline &pipeline);
void cleanupPipeline(DeviceContext &deviceContext, VulkanPipeline &pipeline);
void initRenderer(DeviceContext &deviceContext, SwapchainContext &swapchainContext, Renderer &renderer);
void cleanupRenderer(DeviceContext &deviceContext, Renderer &renderer);
void initMesh(DeviceContext &deviceContext, Mesh &mesh, const Vertex *vertices, uint32_t vertexCount, const uint32_t *indices, uint32_t indexCount);
void cleanupMesh(DeviceContext &deviceContext, Mesh &mesh);
