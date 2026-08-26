#ifndef VULKANINSTANCE_H
#define VULKANINSTANCE_H
#include "Vulkan.h" // IWYU pragma: keep
#include <cstdint>
#include <string>
#include <vector>
class VulkanInstance{
    private:
    const std::string appName = "MyVulkanApp";
    const std::string engineName = "FSEngine";
    const uint32_t appVersion = VK_MAKE_VERSION(1,0,0);
    const uint32_t engineVersion = VK_MAKE_VERSION(1,0,0);
    const uint32_t apiVersion = VK_MAKE_API_VERSION(0,1,3,0);
    const std::vector<const char *> enabledLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char *> enabledExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        #ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        #elif __linux__
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
        #endif
        "VK_EXT_debug_utils"
    };
    VkInstance m_instance = VK_NULL_HANDLE;
    public:
    VulkanInstance();
    ~VulkanInstance();
    VkPhysicalDevice getPhysicalDevice(uint32_t index=0);
    inline VkInstance get(){return m_instance;}
};
#endif