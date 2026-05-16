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
    VkImage                         m_image;

    //TODO(Abstract away window platform-specific)
    #ifdef _WIN32
    HWND                            m_window;
    #elif __linux__
    Display*                        m_display;
    Window                          m_window;
    #endif
    const std::vector<const char*> instanceLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> instanceExtensions = {
        "VK_KHR_surface",
        #ifdef _WIN32
        "VK_KHR_win32_surface"
        #elif __linux__
        "VK_KHR_xlib_surface"
        #endif
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
