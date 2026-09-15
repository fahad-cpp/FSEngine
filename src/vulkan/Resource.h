#ifndef RESOURCE_H
#define RESOURCE_H
#include "../Matrix.h"
#include "../Model.h"
#include "DeviceContext.h"
#include <string>

struct UniformBufferData {
    alignas(16) Matrix4 model;
    alignas(16) Matrix4 view;
    alignas(16) Matrix4 projection;
};
struct Image {
    VkImage        image;
    VkDeviceMemory memory;
};
struct Buffer {
    VkBuffer       buffer;
    VkDeviceMemory memory;
};
struct Texture {
    Image       image;
    VkImageView imageView;
};
struct Mesh {
    Texture  texture;
    Buffer   vertexBuffer;
    Buffer   indexBuffer;
    uint32_t vertexCount;
    uint32_t indexCount;
};

// buffer
Buffer createBuffer(DeviceContext &deviceContext, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memoryProperty);
void   cleanupBuffer(DeviceContext &deviceContext, Buffer &buffer);
Buffer createVertexBuffer(DeviceContext &deviceContext, const Vertex *vertices, uint32_t vertexCount);
Buffer createIndexBuffer(DeviceContext &deviceContext, const uint32_t *indices, uint32_t indexCount);
Buffer createUniformBuffer(DeviceContext &deviceContext, void **pMapped);
// image / texture
Image       createImage(DeviceContext &deviceContext, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryFlags);
void        cleanupImage(DeviceContext &deviceContext, Image &image);
VkImageView createImageView(DeviceContext &deviceContext, VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, uint32_t mipLevels);
VkSampler   createTextureSampler(DeviceContext &deviceContext);
void        generateMipMaps(VkCommandBuffer commandBuffer, VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
Texture     createTexture(DeviceContext &deviceContext, const std::string &filepath);
void        cleanupTexture(DeviceContext &deviceContext, Texture &texture);
// mesh
Mesh createMesh(DeviceContext &deviceContext, OBJModel &model, const std::string &texturePath);
void cleanupMesh(DeviceContext &deviceContext, Mesh &mesh);
#endif