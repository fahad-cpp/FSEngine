#define STB_IMAGE_IMPLEMENTATION
#include "Resource.h"
#include "../Logging.h"
#include "VulkanUtils.h"
#include "stb_image.h"


Buffer createBuffer(DeviceContext &deviceContext, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memoryProperty) {
    Buffer buffer = {};
    VkBufferCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };
    vkCreateBuffer(deviceContext.device, &createInfo, nullptr, &buffer.buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(deviceContext.device, buffer.buffer, &memRequirements);

    uint32_t memoryIndex = getMemoryIndex(deviceContext.physicalDevice, memRequirements, memoryProperty);
    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryIndex
    };
    vkAllocateMemory(deviceContext.device, &memoryAllocateInfo, nullptr, &buffer.memory);
    vkBindBufferMemory(deviceContext.device, buffer.buffer, buffer.memory, 0);

    return buffer;
}
void cleanupBuffer(DeviceContext &deviceContext, Buffer &buffer) {
    vkDestroyBuffer(deviceContext.device, buffer.buffer, nullptr);
    vkFreeMemory(deviceContext.device, buffer.memory, nullptr);
}
Buffer createVertexBuffer(DeviceContext &deviceContext, const Vertex *vertices, uint32_t vertexCount) {
    Buffer vertexBuffer = {};
    if (vertexCount == 0) {
        return {};
    }
    std::size_t bufferSize = sizeof(vertices[0]) * vertexCount;
    Buffer stagingBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *data = nullptr;
    vkMapMemory(deviceContext.device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertices, bufferSize);
    vkUnmapMemory(deviceContext.device, stagingBuffer.memory);

    vertexBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer commandBuffer = startOneTimeCommandBuffer(deviceContext);
    copyBuffer(commandBuffer, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);
    endOneTimeCommandBuffer(deviceContext, commandBuffer);

    vkDestroyBuffer(deviceContext.device, stagingBuffer.buffer, nullptr);
    vkFreeMemory(deviceContext.device, stagingBuffer.memory, nullptr);
    return vertexBuffer;
}
Buffer createIndexBuffer(DeviceContext &deviceContext, const uint32_t *indices, uint32_t indexCount) {
    Buffer indexBuffer = {};
    if (indexCount == 0) {
        return {};
    }
    std::size_t bufferSize = sizeof(indices[0]) * indexCount;
    Buffer stagingBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *data = nullptr;
    vkMapMemory(deviceContext.device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    std::memcpy(data, indices, bufferSize);
    vkUnmapMemory(deviceContext.device, stagingBuffer.memory);

    indexBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer commandBuffer = startOneTimeCommandBuffer(deviceContext);
    copyBuffer(commandBuffer, stagingBuffer.buffer, indexBuffer.buffer, bufferSize);
    endOneTimeCommandBuffer(deviceContext, commandBuffer);

    vkDestroyBuffer(deviceContext.device, stagingBuffer.buffer, nullptr);
    vkFreeMemory(deviceContext.device, stagingBuffer.memory, nullptr);
    return indexBuffer;
}
Buffer createUniformBuffer(DeviceContext &deviceContext, void **pMapped) {
    Buffer uniformBuffer = {};
    std::size_t bufferSize = sizeof(UniformBufferData);

    uniformBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, bufferSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    *pMapped = nullptr;
    vkMapMemory(deviceContext.device, uniformBuffer.memory, 0, bufferSize, 0, &*pMapped);
    return uniformBuffer;
}
Image createImage(DeviceContext &deviceContext, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryFlags) {
    Image image = {};
    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = { width, height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    vkCreateImage(deviceContext.device, &imageInfo, nullptr, &image.image);
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(deviceContext.device, image.image, &memRequirements);
    uint32_t memoryIndex = getMemoryIndex(deviceContext.physicalDevice, memRequirements, memoryFlags);
    VkMemoryAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryIndex
    };
    vkAllocateMemory(deviceContext.device, &allocateInfo, nullptr, &image.memory);
    vkBindImageMemory(deviceContext.device, image.image, image.memory, 0);

    return image;
}
void cleanupImage(DeviceContext &deviceContext, Image &image) {
    vkDestroyImage(deviceContext.device, image.image, nullptr);
    vkFreeMemory(deviceContext.device, image.memory, nullptr);
}
VkImageView createImageView(DeviceContext &deviceContext, VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        // VK_COMPONENT_SWIZZLE_IDENTITY for all components
        .components = {},
        .subresourceRange = {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1 }
    };
    VkImageView imageView = VK_NULL_HANDLE;
    vkCreateImageView(deviceContext.device, &createInfo, nullptr, &imageView);
    return imageView;
}
VkSampler createTextureSampler(DeviceContext &deviceContext) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(deviceContext.physicalDevice, &physicalDeviceProperties);
    VkSamplerCreateInfo samplerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.f,
        .maxLod = 0.f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };
    VkSampler sampler = VK_NULL_HANDLE;
    vkCreateSampler(deviceContext.device, &samplerCreateInfo, nullptr, &sampler);
    return sampler;
}
Texture createTexture(DeviceContext &deviceContext, const std::string &filepath) {
    Texture texture = {};
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4);

    if (!pixels) {
        LOG_ERROR("Failed to load texture: " << filepath);
    }

    Buffer stagingBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, imageSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    void *data = nullptr;
    vkMapMemory(deviceContext.device, stagingBuffer.memory, 0, imageSize, 0, &data);
    std::memcpy(data, pixels, imageSize);
    vkUnmapMemory(deviceContext.device, stagingBuffer.memory);

    stbi_image_free(pixels);

    texture.image = createImage(deviceContext, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer commandBuffer = startOneTimeCommandBuffer(deviceContext);
    transitionImageLayout(
        texture.image.image,
        commandBuffer,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_ACCESS_2_NONE,
        VK_ACCESS_2_TRANSFER_WRITE_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT);
    copyBufferToImage(commandBuffer, stagingBuffer.buffer, texture.image.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(
        texture.image.image,
        commandBuffer,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_TRANSFER_WRITE_BIT,
        VK_ACCESS_2_SHADER_READ_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT);
    endOneTimeCommandBuffer(deviceContext, commandBuffer);

    vkDestroyBuffer(deviceContext.device, stagingBuffer.buffer, nullptr);
    vkFreeMemory(deviceContext.device, stagingBuffer.memory, nullptr);

    texture.imageView = createImageView(deviceContext, texture.image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    texture.sampler = createTextureSampler(deviceContext);
    return texture;
}
void cleanupTexture(DeviceContext &deviceContext, Texture &texture) {
    vkDestroySampler(deviceContext.device, texture.sampler, nullptr);
    vkDestroyImage(deviceContext.device, texture.image.image, nullptr);
    vkFreeMemory(deviceContext.device, texture.image.memory, nullptr);
    vkDestroyImageView(deviceContext.device, texture.imageView, nullptr);
}
Mesh createMesh(DeviceContext &deviceContext, OBJModel &model) {
    Mesh mesh = {};
    mesh.vertexCount = static_cast<uint32_t>(model.vertices.size());
    mesh.indexCount = static_cast<uint32_t>(model.indices.size());
    mesh.vertexBuffer = createVertexBuffer(deviceContext, model.vertices.data(), mesh.vertexCount);
    mesh.indexBuffer = createIndexBuffer(deviceContext, model.indices.data(), mesh.indexCount);
    mesh.texture = createTexture(deviceContext, "textures/viking_room.png");
    return mesh;
}
void cleanupMesh(DeviceContext &deviceContext, Mesh &mesh) {
    cleanupTexture(deviceContext, mesh.texture);

    if (mesh.vertexBuffer.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(deviceContext.device, mesh.vertexBuffer.buffer, nullptr);
        vkFreeMemory(deviceContext.device, mesh.vertexBuffer.memory, nullptr);
    }

    if (mesh.indexBuffer.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(deviceContext.device, mesh.indexBuffer.buffer, nullptr);
        vkFreeMemory(deviceContext.device, mesh.indexBuffer.memory, nullptr);
    }
}