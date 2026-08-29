#include "Buffers.h"
#include "Logging.h"
#include <cstring>

static void copyBuffers(VkQueue queue, VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize) {
    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = bufferSize
    };
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = 0
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr
    };
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}
static std::pair<VkBuffer, VkDeviceMemory> createBuffer(VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VulkanPhysicalDevice &physicanDevice) {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = bufferSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };
    VkResult result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
    LOG_CREATION(result, "Buffer");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
    uint32_t memoryIndex = physicanDevice.getMemoryIndex(memRequirements, memoryProperties);
    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryIndex
    };
    vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory);
    vkBindBufferMemory(device, buffer, memory, 0);

    return { buffer, memory };
}
VertexBuffer::VertexBuffer(VulkanDevice &device, VkCommandBuffer &commandBuffer, VulkanPhysicalDevice &physicalDevice, const std::vector<Vertex> &vertices) {
    m_device = device.get();
    m_vertices = vertices;
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(vertices.size() * sizeof(vertices[0]));
    std::tie(m_stagingBuffer, m_stagingBufferMemory) = createBuffer(device.get(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, physicalDevice);

    void *stagingData = nullptr;
    vkMapMemory(device.get(), m_stagingBufferMemory, 0, bufferSize, 0, &stagingData);
    std::memcpy(stagingData, vertices.data(), bufferSize);
    vkUnmapMemory(device.get(), m_stagingBufferMemory);

    std::tie(m_vertexBuffer, m_vertexBufferMemory) = createBuffer(device.get(), bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDevice);
    VkQueue queue = device.getQueue(physicalDevice.getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT));
    copyBuffers(queue, commandBuffer, m_stagingBuffer, m_vertexBuffer, bufferSize);
}
VertexBuffer::~VertexBuffer() {
    vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
    vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);

    vkFreeMemory(m_device, m_stagingBufferMemory, nullptr);
    vkDestroyBuffer(m_device, m_stagingBuffer, nullptr);

    std::cout << "Destroyed Vertex Buffer Data\n";
}

VkVertexInputBindingDescription VertexBuffer::getBindingDescription() {
    return {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}

std::array<VkVertexInputAttributeDescription, 2> VertexBuffer::getAttributeDescription() {
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, pos) },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color) }
    };
}

IndexBuffer::IndexBuffer(VulkanDevice &device, VkCommandBuffer commandBuffer, VulkanPhysicalDevice &physicalDevice, const std::vector<uint32_t> &indices) {
    m_device = device.get();
    m_indices = indices;
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(indices.size() * sizeof(indices[0]));

    std::tie(m_stagingBuffer, m_stagingBufferMemory) = createBuffer(device.get(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, physicalDevice);

    void *stagingData = nullptr;
    vkMapMemory(device.get(), m_stagingBufferMemory, 0, bufferSize, 0, &stagingData);
    std::memcpy(stagingData, indices.data(), bufferSize);
    vkUnmapMemory(device.get(), m_stagingBufferMemory);

    std::tie(m_indexBuffer, m_indexBufferMemory) = createBuffer(device.get(), bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDevice);
    VkQueue queue = device.getQueue(physicalDevice.getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT));
    copyBuffers(queue, commandBuffer, m_stagingBuffer, m_indexBuffer, bufferSize);
}

IndexBuffer::~IndexBuffer() {
    vkFreeMemory(m_device, m_stagingBufferMemory, nullptr);
    vkDestroyBuffer(m_device, m_stagingBuffer, nullptr);
    vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
    vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
    std::cout << "Destroyed Index Buffer Data\n";

}