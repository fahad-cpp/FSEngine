#include "Device.h"
#include "PhysicalDevice.h"
#include <iostream>
#include <vector>
#include "Logging.h"

VulkanDevice::VulkanDevice(VulkanPhysicalDevice &physicalDevice, const std::vector<VkQueueFlags> &queueFlags) {
    // TODO:Fix same queue index appearing twice for different flags
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(queueFlags.size());
    VkPhysicalDeviceFeatures enabledFeatures = {

    };
    VkPhysicalDeviceVulkan13Features v13features = {};
    v13features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    v13features.synchronization2 = VK_TRUE;
    v13features.dynamicRendering = VK_TRUE;
    
    float priority = 0.5f;
    for (uint32_t i = 0; i < queueFlags.size(); ++i) {
        VkDeviceQueueCreateInfo queueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = physicalDevice.getQueueFamilyIndex(queueFlags[i]),
            .queueCount = 1,
            .pQueuePriorities = &priority
        };
        queueCreateInfos[i] = queueCreateInfo;
    }
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &v13features,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queueFlags.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
        .ppEnabledExtensionNames = enabledExtensions.data(),
        .pEnabledFeatures = &enabledFeatures
    };
    VkResult result = vkCreateDevice(physicalDevice.get(), &deviceCreateInfo, nullptr, &m_device);
    LOG_CREATION(result,"Device");
}

VulkanDevice::~VulkanDevice() {
    this->waitIdle();
    if (m_semaphores.size()) {
        for (VkSemaphore &semaphore : m_semaphores) {
            vkDestroySemaphore(m_device, semaphore, nullptr);
        }
    }
    if (m_fences.size()) {
        for (VkFence &fence : m_fences) {
            vkDestroyFence(m_device, fence, nullptr);
        }
    }
    vkDestroyDevice(m_device, nullptr);
    std::cout << "Destroyed Device\n";
}
VkQueue VulkanDevice::getQueue(const uint32_t familyIndex) {
    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device, familyIndex, 0, &queue);
    return queue;
}
std::vector<VkSemaphore> VulkanDevice::createSemaphores(uint32_t count, const VkSemaphoreCreateFlags flags) {
    VkSemaphoreCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags
    };
    std::vector<VkSemaphore> semaphores(count);
    for (uint32_t i = 0; i < count; ++i) {
        vkCreateSemaphore(m_device, &createInfo, nullptr, &semaphores[i]);
    }
    m_semaphores.reserve(m_semaphores.size() + count);
    for (VkSemaphore &semaphore : semaphores) {
        m_semaphores.push_back(semaphore);
    }
    return semaphores;
}
std::vector<VkFence> VulkanDevice::createFences(uint32_t count, const VkFenceCreateFlags flags) {
    VkFenceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags
    };
    std::vector<VkFence> fences(count);
    for (uint32_t i = 0; i < count; ++i) {
        vkCreateFence(m_device, &createInfo, nullptr, &fences[i]);
    }
    m_fences.reserve(m_fences.size() + count);
    for (VkFence &fence : fences) {
        m_fences.push_back(fence);
    }
    return fences;
}

void VulkanDevice::waitIdle() {
    vkDeviceWaitIdle(m_device);
}