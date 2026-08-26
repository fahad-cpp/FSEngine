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
  uint32_t m_frameIndex=0;
  BackBufferState m_backBufferState = {};
  VulkanDevice* m_pDevice = nullptr;
  VulkanSwapchain* m_pSwapchain = nullptr;
  VkPipeline m_pipeline = VK_NULL_HANDLE;
  VkQueue m_queue = VK_NULL_HANDLE;
  VertexBuffer* m_pVertexBuffer = nullptr;
  IndexBuffer* m_pIndexBuffer = nullptr;
  public:
    Renderer(VulkanDevice &device,VulkanPhysicalDevice& physicalDevice, VulkanSwapchain &swapchain, BackBufferState &backBufferState, VulkanPipeline &pipeline);
    ~Renderer();
    void renderCommands(uint32_t imageIndex);
    void drawFrame();
    void setVertices(VertexBuffer& vertexBuffer);
    void setIndices(IndexBuffer& indexBuffer);
};
#endif