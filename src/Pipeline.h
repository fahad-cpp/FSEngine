#ifndef VULKANPIPELINE_H
#define VULKANPIPELINE_H
#include "Buffers.h"
#include "Swapchain.h"
#include <string>
class VulkanPipeline {
  private:
    VkPipeline m_pipeline;
    VkDevice m_device;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

  public:
    VulkanPipeline(VulkanDevice &device, VulkanSwapchain &swapchain, const std::string &shaderPath, VertexBuffer &vertexBuffer, IndexBuffer &indexBuffer);
    ~VulkanPipeline();
    inline VkPipeline get() { return m_pipeline; }
};
#endif