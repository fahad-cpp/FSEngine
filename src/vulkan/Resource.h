#ifndef RESOURCE_H
#define RESOURCE_H
#include "../Matrix.h"
#include "../Model.h"
#include "DeviceContext.h"
#include <string>

struct UniformBufferData {
    alignas(16) Matrix4 view;
    alignas(16) Matrix4 projection;
};
struct EntityBufferData {
    Matrix4 model;
    uint32_t textureIndex;
    uint32_t padding[3];
};
struct Image {
    VkImage image;
    VkDeviceMemory memory;
};
struct Buffer {
    VkBuffer bufferHandle;
    VkDeviceMemory memoryHandle;
};
struct MappedBuffer {
    Buffer buffer;
    void *mapped;
};
struct VulkanEntity {
    uint32_t meshIndex;
};
struct Texture {
    Image image;
    VkImageView imageView;
};
struct VulkanMesh {
    Texture texture;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    uint32_t vertexCount;
    uint32_t indexCount;
};

// buffer
Buffer createBuffer(DeviceContext &deviceContext, const VkBufferUsageFlags usage, const VkDeviceSize size, const VkMemoryPropertyFlags memoryProperty);
void cleanupBuffer(DeviceContext &deviceContext, Buffer &buffer);
Buffer createVertexBuffer(DeviceContext &deviceContext, const Vertex *vertices, uint32_t vertexCount);
Buffer createIndexBuffer(DeviceContext &deviceContext, const uint32_t *indices, uint32_t indexCount);
MappedBuffer createUniformBuffer(DeviceContext &deviceContext);
MappedBuffer createEntityStorageBuffer(DeviceContext &deviceContext, const uint32_t entityCount);
Buffer createIndirectDrawBuffer(DeviceContext &deviceContext, const uint32_t entityCount);
void uploadToDeviceBuffer(DeviceContext &deviceContext, const Buffer buffer, const void *data, const std::size_t size);
// image / texture
Image createImage(DeviceContext &deviceContext, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryFlags);
void cleanupImage(DeviceContext &deviceContext, Image &image);
VkImageView createImageView(DeviceContext &deviceContext, VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, uint32_t mipLevels);
VkSampler createTextureSampler(DeviceContext &deviceContext);
void generateMipMaps(VkCommandBuffer commandBuffer, VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
Texture createTexture(DeviceContext &deviceContext, const std::string &filepath);
void cleanupTexture(DeviceContext &deviceContext, Texture &texture);
// mesh
VulkanMesh createMesh(DeviceContext &deviceContext, Model &model);
void cleanupMesh(DeviceContext &deviceContext, VulkanMesh &mesh);
#endif