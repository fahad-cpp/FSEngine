#include "Vulkan.h" // IWYU pragma: keep
#include "FSWindow.h"
#include "Vector.h"

#define SELECTED_DEVICE 0

struct Buffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
};
struct Vertex {
    Vector2 pos;
    Vector3 color;
};
struct DeviceContext {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    uint32_t graphicsFamilyIndex;
};

VkResult createInstance(DeviceContext &deviceContext);
VkResult getPhysicalDevice(DeviceContext &deviceContext);
VkResult createDevice(DeviceContext &deviceContext);
void getQueue(DeviceContext &deviceContext);
VkResult createCommandPool(DeviceContext &deviceContext);
VkResult createCommandBuffers(DeviceContext &deviceContext, uint32_t count, VkCommandBuffer *cmdBuffers);
VkResult createSurface(DeviceContext &deviceContext, FS::Window &windowHandle);
VkResult createSemaphores(VkDevice &device, uint32_t count, VkSemaphore *semaphores);
VkResult createSemaphore(VkDevice &device, VkSemaphore *semaphore);
VkResult createFences(VkDevice &device, uint32_t count, VkFence *fences, VkFenceCreateFlags flags = 0);
VkResult createFence(VkDevice &device, VkFence *fence, VkFenceCreateFlags flags = 0);
uint32_t getMemoryIndex(VkPhysicalDevice &physicalDevice, VkMemoryRequirements requirements, VkMemoryPropertyFlags requiredFlags);
Buffer createBuffer(DeviceContext &deviceContext, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memoryProperty);
void copyBuffer(DeviceContext &deviceContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void createVertexBuffer(DeviceContext &deviceContext, const Vertex *vertices, uint32_t vertexCount, Buffer &vertexBuffer);
void createIndexBuffer(DeviceContext &deviceContext, const uint32_t *indices, uint32_t indexCount, Buffer &indexBuffer);
void initDeviceContext(DeviceContext &deviceContext, FS::Window &window);
void cleanupDeviceContext(DeviceContext &deviceContext);