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
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkExtent2D m_swapchainExtent = {};
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkBufferView m_bufferView = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkSemaphore m_imageAcquireSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence m_drawFence = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages = {};
    std::vector<VkImageView> m_swapchainImageViews = {};
    std::vector<VkImage> m_images = {};
    std::vector<VkImageView> m_imageViews = {};
    std::vector<VkDeviceMemory> m_memory = {};

    const std::vector<const char *> layers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char *> instanceExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif __linux__
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
        "VK_EXT_debug_utils",
    };

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

  public:
    FS::Window *window = nullptr;
    VulkanCore() {
        init();
    }
    ~VulkanCore() {
        cleanup();
    }

    VulkanCore(const VulkanCore &) = delete;
    VulkanCore &operator=(const VulkanCore &) = delete;
    VulkanCore(const VulkanCore &&) = delete;
    VulkanCore &operator=(const VulkanCore &&) = delete;

    void drawFrame();

  private:
    VkResult createSwapchain(VkSwapchainKHR &swapchain, FS::Window &window);
    void recordCommandBuffer(uint32_t imageIndex);
    int init();
    void cleanup();
};
#endif
