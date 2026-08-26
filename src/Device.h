#ifndef VULKANDEVICE_H
#define VULKANDEVICE_H
#include "PhysicalDevice.h"
#include <cstdint>
#include <vector>
class VulkanDevice {
  private:
    VkDevice m_device;
    std::vector<VkSemaphore> m_semaphores = {};
    std::vector<VkFence> m_fences = {};
    const std::vector<const char *> enabledExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

  public:
    VulkanDevice(VulkanPhysicalDevice &physicalDevice, const std::vector<VkQueueFlags> &queueFlags);
    ~VulkanDevice();
    VkQueue getQueue(const uint32_t familyIndex);
    std::vector<VkSemaphore> createSemaphores(uint32_t count, const VkSemaphoreCreateFlags flags = 0);
    std::vector<VkFence> createFences(uint32_t count, const VkFenceCreateFlags flags = 0);
    void waitIdle();
    inline VkDevice get() { return m_device; }
};
#endif