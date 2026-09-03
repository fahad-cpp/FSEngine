#include "FSWindow.h"
#include "Vulkan.h" // IWYU pragma: keep


#define SELECTED_DEVICE 0
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
void copyBuffer(DeviceContext &deviceContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void initDeviceContext(DeviceContext &deviceContext, FS::Window &window);
void cleanupDeviceContext(DeviceContext &deviceContext);