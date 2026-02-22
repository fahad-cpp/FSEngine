#ifndef VULKANCORE_H
#define VULKANCORE_H
#include "Window.h"
#include <vulkan/vulkan.h>
#include <vector>
class VulkanCore {
    VkInstance                      m_instance;
    VkPhysicalDevice                m_physicalDevice;
    VkDevice                        m_device;
    VkSurfaceKHR                    m_surface;
    VkSwapchainKHR                  m_swapchain;
    VkBuffer                        m_buffer = VK_NULL_HANDLE;
    std::vector<VkImage>            m_swapchainImages;
    std::vector<VkImageView>        m_swapchainImageViews;

    //TODO(Abstract away window platform-specific)
    HWND                            m_window;
    const std::vector<const char*> instanceLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> instanceExtensions = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface"
    };

    const std::vector<const char*> deviceLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        "VK_KHR_swapchain"
    };
public:
    VulkanCore() {
        init();
    }
    ~VulkanCore() {
        cleanup();
    }

private:
    int init();
    void cleanup();
};
#endif