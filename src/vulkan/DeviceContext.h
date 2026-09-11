#ifndef DEVICECONTEXT_H
#define DEVICECONTEXT_H
#include "FSWindow.h"
#include "Vulkan.h" // IWYU pragma: keep

#define SELECTED_DEVICE 0

struct DeviceContext {
    VkDebugUtilsMessengerEXT debugMessenger;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
};
// device specific
VkInstance createInstance(DeviceContext &deviceContext);
VkPhysicalDevice getPhysicalDevice(DeviceContext &deviceContext);
uint32_t getQueueFamilyIndex(DeviceContext &deviceContext, VkQueueFlags queueFlags);
uint32_t getMemoryIndex(VkPhysicalDevice &physicalDevice, VkMemoryRequirements requirements, VkMemoryPropertyFlags requiredFlags);
VkDevice createDevice(DeviceContext &deviceContext);
VkQueue getQueue(DeviceContext &deviceContext, VkQueueFlags queueFlags);
VkSurfaceKHR createSurface(DeviceContext &deviceContext, FS::Window &windowHandle);
// command buffer/pool
VkCommandPool createCommandPool(DeviceContext &deviceContext, VkQueueFlags queueFlags);
VkResult createCommandBuffers(DeviceContext &deviceContext, uint32_t count, VkCommandBuffer *cmdBuffers);
VkCommandBuffer startOneTimeCommandBuffer(DeviceContext &deviceContext);
void endOneTimeCommandBuffer(DeviceContext &deviceContext, VkCommandBuffer &commandBuffer);
// synchronization
VkResult createSemaphores(VkDevice &device, uint32_t count, VkSemaphore *semaphores);
VkResult createSemaphore(VkDevice &device, VkSemaphore *semaphore);
VkResult createFences(VkDevice &device, uint32_t count, VkFence *fences, VkFenceCreateFlags flags = 0);
VkResult createFence(VkDevice &device, VkFence *fence, VkFenceCreateFlags flags = 0);
// copy commands
void copyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height);
// device context
void initDeviceContext(DeviceContext &deviceContext, FS::Window &window);
void cleanupDeviceContext(DeviceContext &deviceContext);
#endif