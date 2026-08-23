#ifndef VULKANCORE_H
#define VULKANCORE_H

#include <vulkan/vulkan_core.h>
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <FSWindow.h>
#include <vector>
#include <vulkan/vulkan.h>
#define MAX_FRAMES_IN_FLIGHT 2
class VulkanCore {
  public:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkSurfaceFormatKHR m_surfaceFormat = {};
    VkExtent2D m_swapchainExtent = {};
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers = {};
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages = {};
    std::vector<VkImageView> m_swapchainImageViews = {};
    std::vector<VkDeviceMemory> m_memory = {};
    // Synchronization
    std::vector<VkSemaphore> m_imageAcquireSemaphores = {};
    std::vector<VkSemaphore> m_renderFinishedSemaphores = {};
    std::vector<VkFence> m_inFlightFences = {};

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

  private:
    uint32_t frameIndex = 0;

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
    void createSwapchain();
    void createSwapchainImageViews();
    void cleanupSwapchain();
    void recreateSwapchain();
    void createGraphicsPipeline(const std::string &shaderPath);
    void recordCommandBuffer(uint32_t imageIndex);
    int init();
    void cleanup();
};
#endif
