#ifndef VULKANBUFFERS_H
#define VULKANBUFFERS_H
#include "Device.h"
#include "Vector.h"
#include <cstdint>
#include <vector>
struct Vertex {
    Vec2 pos;
    Vec3 color;
};
class VertexBuffer {
  private:
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_memory;
    VkDevice m_device;
    std::vector<Vertex> m_vertices;

  public:
    VertexBuffer(VulkanDevice &device, VulkanPhysicalDevice &physicalDevice, const std::vector<Vertex> &vertices);
    ~VertexBuffer();
    VkVertexInputBindingDescription getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription();
    inline uint32_t getVertexCount() { return static_cast<uint32_t>(m_vertices.size()); };
    inline VkBuffer get() { return m_vertexBuffer; }
};

class IndexBuffer {
  private:
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_memory;
    VkDevice m_device;
    std::vector<uint32_t> m_indices;

  public:
    IndexBuffer(VulkanDevice &device, VulkanPhysicalDevice &physicalDevice, const std::vector<uint32_t> &indices);
    ~IndexBuffer();
    inline VkBuffer get() { return m_indexBuffer; }
};
#endif