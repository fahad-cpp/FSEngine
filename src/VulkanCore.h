#ifndef VULKANCORE_H
#define VULKANCORE_H

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <FSWindow.h>
#include <vector>
#include <vulkan/vulkan.h>
class VulkanCore {
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapchain;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkBufferView m_bufferView;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkImage> m_images = {};
    std::vector<VkImageView> m_imageViews = {};
    std::vector<VkDeviceMemory> m_memory = {};
    FS::Window *m_window;

    const std::vector<const char *> instanceLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char *> instanceExtensions = {
        "VK_KHR_surface",
#ifdef _WIN32
        "VK_KHR_win32_surface"
#elif __linux__
        "VK_KHR_xlib_surface"
#endif
    };

    const std::vector<const char *> deviceLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char *> deviceExtensions = {
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
