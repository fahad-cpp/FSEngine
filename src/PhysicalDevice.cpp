#include "PhysicalDevice.h"
#include <cstdint>
#include <iostream>
#include <vector>

VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice physicalDevice) {
    m_physicalDevice = physicalDevice;
}
VulkanPhysicalDevice::~VulkanPhysicalDevice() {
}

uint32_t VulkanPhysicalDevice::getQueueFamilyIndex(VkQueueFlags flags) {
    uint32_t propertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &propertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> familyProperties(propertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &propertyCount, familyProperties.data());

    uint32_t familyIndex = ~0u;
    for (uint32_t i = 0; i < familyProperties.size(); ++i) {
        if ((familyProperties[i].queueFlags & flags) == flags) {
            familyIndex = i;
            break;
        } else if (i == (familyProperties.size() - 1)) {
            std::cerr << "Failed to get any queue family with flags:" << flags << "\n";
        }
    }

    return familyIndex;
}

uint32_t VulkanPhysicalDevice::getMemoryIndex(VkMemoryRequirements requirements, VkMemoryPropertyFlags requiredFlags) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    uint32_t memoryIndex = ~0u;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if(!(requirements.memoryTypeBits & (1 << i)))continue;
        if (((memProperties.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags)) {
            memoryIndex = i;
            break;
        }
    }
    if(memoryIndex == ~0u){
        std::cerr << "Could not find memory index with given flags:" << requiredFlags << "\n";
        std::cerr << "Falling back to index 0\n";
        return 0;
    }

    return memoryIndex;
}