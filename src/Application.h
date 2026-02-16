#ifndef APPLICATION
#define APPLICATION
#ifdef _WIN32
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>
#include <vector>
#include "Allocator.h"
class Application {
    VkInstance                      m_instance;
    VkPhysicalDevice                m_physicalDevice;
    VkDevice                        m_device;
    VkSurfaceKHR                    m_surface;
    VkSwapchainKHR                  m_swapchain;
    VkBuffer                        m_buffer = VK_NULL_HANDLE;
    const char* instanceLayers[1] = {
        "VK_LAYER_KHRONOS_validation"
    };
    const char* instanceExtensions[2] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface"
    };

    const char* deviceLayers[1] = {
        "VK_LAYER_KHRONOS_validation"
    };

    const char* deviceExtensions[1] = {
        "VK_KHR_swapchain"
    };
public:

    int         init();
    void        run();
    void        cleanup();
};
#endif