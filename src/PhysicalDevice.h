#ifndef VULKANPHYSICALDEVICE_H
#define VULKANPHYSICALDEVICE_H
#include "Vulkan.h" // IWYU pragma: keep
class VulkanPhysicalDevice {
  private:
    VkPhysicalDevice m_physicalDevice;

  public:
    VulkanPhysicalDevice(VkPhysicalDevice physicalDevice);
    ~VulkanPhysicalDevice();
    uint32_t getQueueFamilyIndex(VkQueueFlags flags);
    uint32_t getMemoryIndex(VkMemoryRequirements requirements, VkMemoryPropertyFlags requiredFlags = 0);
    inline VkPhysicalDevice get() { return m_physicalDevice; }
};
#endif