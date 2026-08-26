#ifndef VULKANCOMMAND_H
#define VULKANCOMMAND_H
#include "Device.h"
#include <cstdint>
#include <vector>
class VulkanCommandPool{
    private:
    VkCommandPool m_commandPool;
    VkDevice m_device;
    public:
    VulkanCommandPool(VulkanDevice& device,uint32_t queueFamilyIndex,VkCommandPoolCreateFlags flags=0);
    ~VulkanCommandPool();
    std::vector<VkCommandBuffer> createCommandBuffers(uint32_t count=1);
};
#endif