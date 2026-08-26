#include "Logging.h"
#include "Surface.h"

VulkanSurface::VulkanSurface(VulkanInstance &instance, VulkanPhysicalDevice &physicalDevice, FS::Window &window) {
    m_instance = instance.get();
    m_physicalDevice = physicalDevice.get();
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .hinstance = GetModuleHandleA(nullptr),
        .hwnd = window.getNative()
    };
    VkResult result = vkCreateWin32SurfaceKHR(instance.get(), &surfaceInfo, nullptr, &m_surface);
#elif __linux
    VkXlibSurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .dpy = XOpenDisplay(nullptr),
        .window = window.getNative()
    };
    VkResult result = vkCreateXlibSurfaceKHR(instance.get(), &surfaceInfo, nullptr, &m_surface);
#endif
    LOG_CREATION(result,"Surface");
}
VulkanSurface::~VulkanSurface() {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    std::cout << "Destroyed Surface\n";
}

VkSurfaceCapabilitiesKHR VulkanSurface::getCapabilities() {
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCaps);
    return surfaceCaps;
}

std::vector<VkSurfaceFormatKHR> VulkanSurface::getFormats() {
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, surfaceFormats.data());

    return surfaceFormats;
}

std::vector<VkPresentModeKHR> VulkanSurface::getPresentModes(){
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data());

    return presentModes;
}