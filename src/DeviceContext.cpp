#include "DeviceContext.h"

VkResult createInstance(DeviceContext &deviceContext) {
    // TODO: check for layers support
    const char *instanceLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    const char *instanceExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif __linux__
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    // Application Info
    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "MyApp",
        .applicationVersion = 1,
        .pEngineName = "FSEngine",
        .engineVersion = 1,
        .apiVersion = VK_MAKE_API_VERSION(0, 1, 4, 0)
    };

    // Instance Create Info
    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = sizeof(instanceLayers) / sizeof(instanceLayers[0]),
        .ppEnabledLayerNames = instanceLayers,
        .enabledExtensionCount = sizeof(instanceExtensions) / sizeof(instanceExtensions[0]),
        .ppEnabledExtensionNames = instanceExtensions
    };

    // Create the Instance
    return vkCreateInstance(&instanceCreateInfo, nullptr, &deviceContext.instance);
}
VkResult getPhysicalDevice(DeviceContext &deviceContext) {
    // Enumerate Device
    uint32_t physicalDeviceCount = 16;
    VkPhysicalDevice physicalDevices[16];
    VkResult result = vkEnumeratePhysicalDevices(deviceContext.instance, &physicalDeviceCount, physicalDevices);
    // Select a device
    deviceContext.physicalDevice = physicalDevices[SELECTED_DEVICE];
    return result;
}
VkResult createDevice(DeviceContext &deviceContext) {

    const char *deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(deviceContext.physicalDevice, &supportedFeatures);

    // Get supported Modern vulkan features
    VkPhysicalDeviceVulkan13Features v13features{};
    v13features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    VkPhysicalDeviceFeatures2 supportedFeatures2{};
    supportedFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    supportedFeatures2.pNext = &v13features;
    vkGetPhysicalDeviceFeatures2(deviceContext.physicalDevice, &supportedFeatures2);

    // Specify required Features
    // An Example where tesselation shader and geometry shaders are must have
    // and multiDrawIndirect is supported if the device supports it
    VkPhysicalDeviceFeatures requiredFeatures{};
    requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
    requiredFeatures.sparseBinding = supportedFeatures.sparseBinding;
    requiredFeatures.sparseResidencyImage2D = supportedFeatures.sparseResidencyImage2D;
    requiredFeatures.tessellationShader = VK_TRUE;
    requiredFeatures.geometryShader = VK_TRUE;

    // Dynamic Rendering required
    VkPhysicalDeviceVulkan13Features requiredFeaturesvk13{};
    requiredFeaturesvk13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    requiredFeaturesvk13.dynamicRendering = VK_TRUE;
    requiredFeaturesvk13.synchronization2 = VK_TRUE;

    uint32_t queueFamilyCount = 16;
    VkQueueFamilyProperties queueFamilyProperties[16];
    vkGetPhysicalDeviceQueueFamilyProperties(deviceContext.physicalDevice, &queueFamilyCount, queueFamilyProperties);
    VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
    uint32_t familyIndex = 0;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags) {
            familyIndex = i;
            break;
        }
    }
    deviceContext.graphicsFamilyIndex = familyIndex;
    // Queue Create Info
    float priority = 1.f;
    const VkDeviceQueueCreateInfo queueCreateInfo[] = {
        { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .queueFamilyIndex = familyIndex,
          .queueCount = 1,
          .pQueuePriorities = &priority },
    };

    // Device Create Info
    const VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &requiredFeaturesvk13,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = queueCreateInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]),
        .ppEnabledExtensionNames = deviceExtensions,
        .pEnabledFeatures = &requiredFeatures
    };

    return vkCreateDevice(deviceContext.physicalDevice, &deviceCreateInfo, nullptr, &deviceContext.device);
}
void getQueue(DeviceContext &deviceContext) {
    vkGetDeviceQueue(deviceContext.device, deviceContext.graphicsFamilyIndex, 0, &deviceContext.graphicsQueue);
}
VkResult createSurface(DeviceContext &deviceContext, FS::Window &windowHandle) {
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .hinstance = GetModuleHandleA(nullptr),
        .hwnd = windowHandle.getNative()
    };
    return vkCreateWin32SurfaceKHR(deviceContext.instance, &surfaceCreateInfo, nullptr, &deviceContext.surface);
#elif __linux__
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .dpy = XOpenDisplay(0),
        .window = windowHandle.getNative()
    };
    return vkCreateXlibSurfaceKHR(deviceContext.instance, &surfaceCreateInfo, nullptr, &deviceContext.surface);
#endif
}

VkResult createCommandPool(DeviceContext &deviceContext) {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = deviceContext.graphicsFamilyIndex
    };

    return vkCreateCommandPool(deviceContext.device, &commandPoolCreateInfo, nullptr, &deviceContext.commandPool);
}
VkResult createCommandBuffers(DeviceContext &deviceContext, uint32_t count, VkCommandBuffer *cmdBuffers) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = deviceContext.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count
    };

    return vkAllocateCommandBuffers(deviceContext.device, &commandBufferAllocateInfo, cmdBuffers);
}
VkResult createSemaphores(VkDevice &device, uint32_t count, VkSemaphore *semaphores) {
    VkSemaphoreCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    VkResult res;
    for (uint32_t i = 0; i < count; ++i) {
        res = vkCreateSemaphore(device, &createInfo, nullptr, &semaphores[i]);
        if (res != VK_SUCCESS) {
            return res;
        }
    }
    return res;
}
VkResult createSemaphore(VkDevice &device, VkSemaphore *semaphore) {
    VkSemaphoreCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    return vkCreateSemaphore(device, &createInfo, nullptr, semaphore);
}
VkResult createFences(VkDevice &device, uint32_t count, VkFence *fences, VkFenceCreateFlags flags) {
    VkFenceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags
    };
    VkResult res;
    for (uint32_t i = 0; i < count; ++i) {
        res = vkCreateFence(device, &createInfo, nullptr, &fences[i]);
        if (res != VK_SUCCESS) {
            return res;
        }
    }
    return res;
}
VkResult createFence(VkDevice &device, VkFence *fence, VkFenceCreateFlags flags) {
    VkFenceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags
    };
    return vkCreateFence(device, &createInfo, nullptr, fence);
}
uint32_t getMemoryIndex(VkPhysicalDevice &physicalDevice, VkMemoryRequirements requirements, VkMemoryPropertyFlags requiredFlags) {
    uint32_t selectedType = ~0u;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; ++memoryType) {
        if (requirements.memoryTypeBits & (1 << memoryType)) {
            const VkMemoryType &type = memoryProperties.memoryTypes[memoryType];
            if ((type.propertyFlags & requiredFlags) == requiredFlags) {
                selectedType = memoryType;
                break;
            }
        }
    }
    return selectedType;
}
void copyBuffer(DeviceContext &deviceContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = deviceContext.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(deviceContext.device, &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };
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
    vkQueueSubmit(deviceContext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(deviceContext.graphicsQueue);
    vkFreeCommandBuffers(deviceContext.device, deviceContext.commandPool, 1, &commandBuffer);
}
void initDeviceContext(DeviceContext &deviceContext, FS::Window &window) {
    createInstance(deviceContext);
    getPhysicalDevice(deviceContext);
    createDevice(deviceContext);
    getQueue(deviceContext);
    createSurface(deviceContext, window);
    createCommandPool(deviceContext);
}
void cleanupDeviceContext(DeviceContext &deviceContext) {
    vkDeviceWaitIdle(deviceContext.device);
    vkDestroyCommandPool(deviceContext.device, deviceContext.commandPool, nullptr);
    vkDestroySurfaceKHR(deviceContext.instance, deviceContext.surface, nullptr);
    vkDestroyDevice(deviceContext.device, nullptr);
    vkDestroyInstance(deviceContext.instance, nullptr);
}