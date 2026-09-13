#include "SwapchainContext.h"
#include <algorithm>
#include <assert.h>
#include <climits>
#include <thread>
#include "../Logging.h"

void createSwapchain(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window) {
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceContext.physicalDevice, deviceContext.surface, &surfaceCaps);

    // Find correct format
    constexpr uint32_t MAX_SURFACE_FORMATS = 64;
    uint32_t formatCount = 0;
    VkSurfaceFormatKHR availableFormats[MAX_SURFACE_FORMATS];
    vkGetPhysicalDeviceSurfaceFormatsKHR(deviceContext.physicalDevice, deviceContext.surface, &formatCount, nullptr);
    assert(formatCount < MAX_SURFACE_FORMATS);
    vkGetPhysicalDeviceSurfaceFormatsKHR(deviceContext.physicalDevice, deviceContext.surface, &formatCount, availableFormats);

    for (uint32_t i = 0; i < formatCount; ++i) {
        if (availableFormats[i].format == VK_FORMAT_R8G8B8A8_SRGB) {
            swapchainContext.surfaceFormat = availableFormats[i];
            break;
        } else if (i == (formatCount - 1)) {
            swapchainContext.surfaceFormat = availableFormats[0];
            break;
        }
    }

    // Find correct present mode
    constexpr uint32_t MAX_PRESENT_MODES = 16;
    uint32_t presentModeCount = MAX_PRESENT_MODES;
    VkPresentModeKHR presentModes[MAX_PRESENT_MODES];
    vkGetPhysicalDeviceSurfacePresentModesKHR(deviceContext.physicalDevice, deviceContext.surface, &presentModeCount, presentModes);

    VkPresentModeKHR selectedMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < presentModeCount; ++i) {
        if (presentModes[i] == selectedMode) {
            break;
        } else if (i == (presentModeCount - 1)) {
            selectedMode = presentModes[0];
            break;
        }
    }

    FS::RenderState &renderState = window.getRenderState();
    while(surfaceCaps.currentExtent.width == 0 || surfaceCaps.currentExtent.height == 0){
        std::this_thread::sleep_for(std::chrono::duration<float,std::chrono::milliseconds::period>(16));
        LOG_LIVE("Window Minimized.");
        window.processMessages();
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceContext.physicalDevice, deviceContext.surface, &surfaceCaps);
    }
    uint32_t windowWidth = std::clamp<uint32_t>(renderState.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
    uint32_t windowHeight = std::clamp<uint32_t>(renderState.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
    swapchainContext.extent = (surfaceCaps.currentExtent.width != UINT_MAX) ? surfaceCaps.currentExtent : VkExtent2D{ windowWidth, windowHeight };
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = deviceContext.surface,
        .minImageCount = surfaceCaps.minImageCount + 1,
        .imageFormat = swapchainContext.surfaceFormat.format,
        .imageColorSpace = swapchainContext.surfaceFormat.colorSpace,
        .imageExtent = swapchainContext.extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = surfaceCaps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = selectedMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    vkCreateSwapchainKHR(deviceContext.device, &swapchainCreateInfo, nullptr, &swapchainContext.swapchain);

    // Swapchain Images
    uint32_t swapchainImageCount = MAX_SWAPCHAIN_IMAGES;
    std::fill_n(swapchainContext.images, MAX_SWAPCHAIN_IMAGES, VK_NULL_HANDLE);
    vkGetSwapchainImagesKHR(deviceContext.device, swapchainContext.swapchain, &swapchainImageCount, swapchainContext.images);
    swapchainContext.imageCount = swapchainImageCount;

    swapchainContext.depth.image = createImage(deviceContext, swapchainContext.extent.width, swapchainContext.extent.height, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    swapchainContext.depth.imageView = createImageView(deviceContext, swapchainContext.depth.image.image, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}
void createSwapchainImageViews(DeviceContext &deviceContext, SwapchainContext &swapchainContext) {
    for (uint32_t i = 0; i < swapchainContext.imageCount; ++i) {
        VkImageView imageView = VK_NULL_HANDLE;
        imageView = createImageView(deviceContext, swapchainContext.images[i], swapchainContext.surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT,1);
        swapchainContext.imageViews[i] = imageView;
    }
}
void initSwapchainContext(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window) {
    createSwapchain(deviceContext, swapchainContext, window);
    createSwapchainImageViews(deviceContext, swapchainContext);
}
void cleanupSwapchainContext(DeviceContext &deviceContext, SwapchainContext &swapchainContext) {
    vkDestroyImageView(deviceContext.device, swapchainContext.depth.imageView, nullptr);
    cleanupImage(deviceContext, swapchainContext.depth.image);
    for (uint32_t i = 0; i < swapchainContext.imageCount; ++i) {
        vkDestroyImageView(deviceContext.device, swapchainContext.imageViews[i], nullptr);
    }
    vkDestroySwapchainKHR(deviceContext.device, swapchainContext.swapchain, nullptr);
    swapchainContext.swapchain = VK_NULL_HANDLE;
}
void recreateSwapchain(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window) {
    vkDeviceWaitIdle(deviceContext.device);
    cleanupSwapchainContext(deviceContext, swapchainContext);
    initSwapchainContext(deviceContext, swapchainContext, window);
}