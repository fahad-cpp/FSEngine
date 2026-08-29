#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H
#include "Device.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include <vector>
struct BackBufferState {
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> acquireSemaphores;
    std::vector<VkSemaphore> renderSemaphores;
    std::vector<VkFence> frameFences;
};

class Renderer {
  private:
    const uint32_t m_framesInFlight = 2;
    uint32_t m_frameIndex = 0;
    BackBufferState m_backBufferState = {};
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;
    uint32_t m_verticesCount = 0;
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    [[maybe_unused]] uint32_t m_indicesCount = 0;
    VkBuffer m_indexBuffer = VK_NULL_HANDLE;

    VulkanSwapchain &m_swapchain;
    VulkanDevice &m_device;

  public:
    Renderer(VulkanDevice &device, VulkanPhysicalDevice &physicalDevice, VulkanSwapchain &swapchain, BackBufferState &backBufferState, VulkanPipeline &pipeline);
    ~Renderer();
    void renderCommands(uint32_t imageIndex);
    void drawFrame();
    void setVertices(VertexBuffer &vertexBuffer);
    void setIndices(IndexBuffer &indexBuffer);
};
#endif