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
VkCommandBuffer startOneTimeCommandBuffer(DeviceContext& deviceContext);
void endOneTimeCommandBuffer(DeviceContext& deviceContext,VkCommandBuffer& commandBuffer);
VkResult createSurface(DeviceContext &deviceContext, FS::Window &windowHandle);
VkResult createSemaphores(VkDevice &device, uint32_t count, VkSemaphore *semaphores);
VkResult createSemaphore(VkDevice &device, VkSemaphore *semaphore);
VkResult createFences(VkDevice &device, uint32_t count, VkFence *fences, VkFenceCreateFlags flags = 0);
VkResult createFence(VkDevice &device, VkFence *fence, VkFenceCreateFlags flags = 0);
VkResult createImageView(DeviceContext& deviceContext,VkImage image,VkFormat format,VkImageView& imageView);
uint32_t getMemoryIndex(VkPhysicalDevice &physicalDevice, VkMemoryRequirements requirements, VkMemoryPropertyFlags requiredFlags);
void copyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void copyBufferToImage(VkCommandBuffer commandBuffer,VkBuffer srcBuffer,VkImage dstImage,uint32_t width,uint32_t height);
void initDeviceContext(DeviceContext &deviceContext, FS::Window &window);
void cleanupDeviceContext(DeviceContext &deviceContext);