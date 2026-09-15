#ifndef SWAPCHAINCONTEXT_H
#define SWAPCHAINCONTEXT_H
#include "DeviceContext.h"
#include "Resource.h"

constexpr uint32_t MAX_SWAPCHAIN_IMAGES = 8;
struct DepthBuffer {
    Image       image;
    VkImageView imageView;
};
struct SwapchainContext {
    VkSwapchainKHR     swapchain;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D         extent;
    uint32_t           imageCount;
    DepthBuffer        depth;
    VkImage            images[MAX_SWAPCHAIN_IMAGES];
    VkImageView        imageViews[MAX_SWAPCHAIN_IMAGES];
};

void createSwapchain(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window);
void createSwapchainImageViews(DeviceContext &deviceContext, SwapchainContext &swapchainContext);
void initSwapchainContext(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window);
void cleanupSwapchainContext(DeviceContext &deviceContext, SwapchainContext &swapchainContext);
void recreateSwapchain(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window);
#endif