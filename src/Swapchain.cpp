#include "Logging.h"
#include "Swapchain.h"
#include <algorithm>
#include <climits>

VulkanSwapchain::VulkanSwapchain(VulkanDevice &device, VulkanSurface &surface, FS::Window &window) : m_surface(surface),m_window(window){
    m_device = device.get();
    this->create();
}
void VulkanSwapchain::create(){
    VkSurfaceCapabilitiesKHR surfaceCaps = m_surface.getCapabilities();
    std::vector<VkSurfaceFormatKHR> surfaceFormats = m_surface.getFormats();
    std::vector<VkPresentModeKHR> presentModes = m_surface.getPresentModes();
    for (uint32_t i = 0; i < surfaceFormats.size(); ++i) {
        if (surfaceFormats[i].format == VK_FORMAT_R8G8B8A8_SRGB) {
            m_surfaceFormat = surfaceFormats[i];
            break;
        } else if (i == (surfaceFormats.size() - 1)) {
            m_surfaceFormat = surfaceFormats[0];
        }
    }

    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    for (uint32_t i = 0; i < presentModes.size(); ++i) {
        if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR) {
            swapchainPresentMode = presentModes[i];
            break;
        } else if (i == (presentModes.size() - 1)) {
            swapchainPresentMode = presentModes[0];
        }
    }
    FS::RenderState &renderState = m_window.getRenderState();
    const uint32_t windowWidth = std::clamp<uint32_t>(renderState.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
    const uint32_t windowHeight = std::clamp<uint32_t>(renderState.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
    m_swapchainExtent = (surfaceCaps.currentExtent.width != UINT_MAX) ? surfaceCaps.currentExtent : VkExtent2D{ windowWidth, windowHeight };
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = m_surface.get(),
        .minImageCount = surfaceCaps.minImageCount,
        .imageFormat = m_surfaceFormat.format,
        .imageColorSpace = m_surfaceFormat.colorSpace,
        .imageExtent = m_swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = surfaceCaps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapchainPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr
    };
    VkResult result = vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain);
    LOG_CREATION(result,"Swapchain");
}
void VulkanSwapchain::recreate(){
    vkDeviceWaitIdle(m_device);
    m_images.clear();
    for(VkImageView& imageView : m_imageViews){
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    m_imageViews.clear();
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    m_swapchain = VK_NULL_HANDLE;

    create();
    getImages();
    getImageViews();
}
VulkanSwapchain::~VulkanSwapchain() {
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    for(VkImageView imageView : m_imageViews){
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    std::cout << "Destroyed Swapchain\n";
}
std::vector<VkImage> VulkanSwapchain::getImages() {
    if(m_images.size())return m_images;
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());

    return m_images;
}
std::vector<VkImageView> VulkanSwapchain::getImageViews() {
    if(m_imageViews.size())return m_imageViews;
    if(!m_images.size())getImages();
    m_imageViews.reserve(m_images.size());
    for(VkImage& image : m_images){
        const VkImageViewCreateInfo imageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_surfaceFormat.format,
            .components = {},
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        VkImageView imageView = VK_NULL_HANDLE;
        vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &imageView);
        m_imageViews.push_back(imageView);
    }

    return m_imageViews;
}