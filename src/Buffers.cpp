#include "Buffers.h"
#include "Logging.h"
#include <cstring>

VertexBuffer::VertexBuffer(VulkanDevice& device,VulkanPhysicalDevice& physicalDevice,const std::vector<Vertex>& vertices){
    m_device = device.get();
    m_vertices = vertices;
    std::size_t bufferSize = (vertices.size() * sizeof(vertices[0]));
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };
    VkResult result = vkCreateBuffer(device.get(), &bufferCreateInfo, nullptr, &m_vertexBuffer);
    LOG_CREATION(result, "Vertex Buffer");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.get(),m_vertexBuffer,&memRequirements);
    uint32_t memoryIndex = physicalDevice.getMemoryIndex(memRequirements,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryIndex
    };
    vkAllocateMemory(device.get(), &memoryAllocateInfo, nullptr, &m_memory);
    vkBindBufferMemory(device.get(), m_vertexBuffer, m_memory, 0);

    void* data;
    vkMapMemory(device.get(), m_memory, 0, bufferSize, 0, &data);
    std::memcpy(data,vertices.data(),bufferSize);
    vkUnmapMemory(device.get(), m_memory);
}
VertexBuffer::~VertexBuffer(){
    vkFreeMemory(m_device, m_memory, nullptr);
    vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    std::cout << "Destroyed Buffer\n";
}

VkVertexInputBindingDescription VertexBuffer::getBindingDescription(){
    return {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}

std::array<VkVertexInputAttributeDescription, 2> VertexBuffer::getAttributeDescription(){
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, pos)
        },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color)
        }
    };
}



IndexBuffer::IndexBuffer(VulkanDevice& device,VulkanPhysicalDevice& physicalDevice,const std::vector<uint32_t>& indices){
    m_device = device.get();
    m_indices = indices;
    std::size_t bufferSize = (indices.size() * sizeof(indices[0]));
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };
    VkResult result = vkCreateBuffer(device.get(), &bufferCreateInfo, nullptr, &m_indexBuffer);
    LOG_CREATION(result, "Index Buffer");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.get(),m_indexBuffer,&memRequirements);
    uint32_t memoryIndex = physicalDevice.getMemoryIndex(memRequirements,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryIndex
    };
    vkAllocateMemory(device.get(), &memoryAllocateInfo, nullptr, &m_memory);
    vkBindBufferMemory(device.get(), m_indexBuffer, m_memory, 0);

    void* data;
    vkMapMemory(device.get(), m_memory, 0, bufferSize, 0, &data);
    std::memcpy(data,indices.data(),bufferSize);
    vkUnmapMemory(device.get(), m_memory);
}

IndexBuffer::~IndexBuffer(){
    vkFreeMemory(m_device, m_memory, nullptr);
    vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
    std::cout << "Destroyed Index Buffer\n";
}