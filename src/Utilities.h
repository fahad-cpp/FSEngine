#ifndef UTILITIES_H
#define UTILITIES_H

#include "Vulkan.h" // IWYU pragma: keep
#include <string>
#include <vector>
#include "Vector.h" // IWYU pragma: keep
#include "Matrix.h" // IWYU pragma: keep


const std::vector<char> readFile(const std::string &path);
void transitionImageLayout(
    VkImage &image,
    VkCommandBuffer &commandBuffer,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkPipelineStageFlags2 srcStageMask,
    VkPipelineStageFlags2 dstStageMask,
    VkAccessFlags2 srcAccessMask,
    VkAccessFlags2 dstAccessMask
);
#endif