#ifndef VULKANUTILS_H
#define VULKANUTILS_H
#include "Vulkan.h" // IWYU pragma: keep

void transitionImageLayout(
    VkImage              &image,
    VkCommandBuffer      &commandBuffer,
    VkImageLayout         oldLayout,
    VkImageLayout         newLayout,
    VkPipelineStageFlags2 srcStageMask,
    VkPipelineStageFlags2 dstStageMask,
    VkAccessFlags2        srcAccessMask,
    VkAccessFlags2        dstAccessMask,
    VkImageAspectFlags    aspectFlags,
    uint32_t              mipLevels);
#endif