#ifndef VULKANSWAPCHAIN_H
#define VULKANSWAPCHAIN_H
#include "Device.h"
#include "Surface.h"
#include <vector>
class VulkanSwapchain {
  private:
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkSurfaceFormatKHR m_surfaceFormat = {};
    VkExtent2D m_swapchainExtent = {};
    std::vector<VkImage> m_images = {};
    std::vector<VkImageView> m_imageViews = {};
    
    VulkanSurface* m_pSurface = nullptr;
    FS::Window* m_pWindow = nullptr;
    void create();
  public:
    VulkanSwapchain(VulkanDevice &device, VulkanSurface &surface, FS::Window &window);
    ~VulkanSwapchain();
    std::vector<VkImage> getImages();
    std::vector<VkImageView> getImageViews();
    void recreate();
    inline VkSurfaceFormatKHR getSurfaceFormat() { return m_surfaceFormat; }
    inline VkExtent2D getExtent() { return m_swapchainExtent; }
    inline VkSwapchainKHR get() { return m_swapchain; }
};
#endif