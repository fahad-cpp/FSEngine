#include "DeviceContext.h"
#include "../Logging.h"
#include "../Scene.h"
#include <array>
#include <assert.h>
#include <string>

static std::string getDebugMessageTypeString(const VkDebugUtilsMessageTypeFlagsEXT messageType) {
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        return "General";
    } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        return "Performance";
    } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        return "Validation";
    } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) {
        return "Address Binding";
    }
    return "InvalidType";
}
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    [[maybe_unused]] void                      *pUserData) {
    const char *severity = "";
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        severity = "VERBOSE";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        severity = "ERROR";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        severity = "WARN";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        severity = "INFO";
    }

    std::string typeString = getDebugMessageTypeString(messageType);
    std::fprintf(stderr, "[VULKAN %s (%s)]:\n%s\n\n", severity, typeString.c_str(), callbackData->pMessage);

    return VK_FALSE;
}
VkInstance createInstance(DeviceContext &deviceContext) {
// TODO: check for layers support
#ifdef _DEBUG
    const char *instanceLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
#endif

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
    const VkApplicationInfo applicationInfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = nullptr,
        .pApplicationName   = "MyApp",
        .applicationVersion = 1,
        .pEngineName        = "FSEngine",
        .engineVersion      = 1,
        .apiVersion         = VK_MAKE_API_VERSION(0, 1, 4, 0)
    };

    const VkValidationFeatureEnableEXT features[] = {
        // VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
        // VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
        // VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
    };

    const VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity =
            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,

        .messageType =
            // VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            // VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,

        .pfnUserCallback = debugCallback,
        .pUserData       = nullptr
    };

    const VkValidationFeaturesEXT validationFeatures = {
        .sType                          = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .pNext                          = &debugUtilsMessengerCreateInfo,
        .enabledValidationFeatureCount  = static_cast<uint32_t>(std::size(features)),
        .pEnabledValidationFeatures     = features,
        .disabledValidationFeatureCount = 0,
        .pDisabledValidationFeatures    = nullptr
    };

    const VkInstanceCreateInfo instanceCreateInfo = {
        .sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext            = &validationFeatures,
        .flags            = 0,
        .pApplicationInfo = &applicationInfo,
#ifdef _DEBUG
        .enabledLayerCount   = sizeof(instanceLayers) / sizeof(instanceLayers[0]),
        .ppEnabledLayerNames = instanceLayers,
#else
        .enabledLayerCount   = 0,
        .ppEnabledLayerNames = nullptr,
#endif
        .enabledExtensionCount   = sizeof(instanceExtensions) / sizeof(instanceExtensions[0]),
        .ppEnabledExtensionNames = instanceExtensions
    };

    // Create the Instance
    VkInstance instance = VK_NULL_HANDLE;
    VkResult   result   = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create instance " << result);
        return VK_NULL_HANDLE;
    }
    const auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfo, nullptr, &deviceContext.debugMessenger);
    return instance;
}
VkPhysicalDevice getPhysicalDevice(DeviceContext &deviceContext) {
    // Enumerate Device
    uint32_t         physicalDeviceCount = 16;
    VkPhysicalDevice physicalDevices[16];
    VkResult         result = vkEnumeratePhysicalDevices(deviceContext.instance, &physicalDeviceCount, physicalDevices);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to select physical device " << result);
        return VK_NULL_HANDLE;
    }

    return physicalDevices[SELECTED_DEVICE];
}
uint32_t getQueueFamilyIndex(DeviceContext &deviceContext, VkQueueFlags queueFlags) {
    uint32_t                queueFamilyCount = 0;
    VkQueueFamilyProperties queueFamilyProperties[16];
    vkGetPhysicalDeviceQueueFamilyProperties(deviceContext.physicalDevice, &queueFamilyCount, nullptr);
    assert(queueFamilyCount < 16);
    vkGetPhysicalDeviceQueueFamilyProperties(deviceContext.physicalDevice, &queueFamilyCount, queueFamilyProperties);
    uint32_t familyIndex = 0;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags) {
            familyIndex = i;
            break;
        }
    }
    return familyIndex;
}
uint32_t getMemoryIndex(VkPhysicalDevice &physicalDevice, VkMemoryRequirements requirements, VkMemoryPropertyFlags requiredFlags) {
    uint32_t                         selectedType = ~0u;
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
VkDevice createDevice(DeviceContext &deviceContext) {

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
    requiredFeatures.multiDrawIndirect      = supportedFeatures.multiDrawIndirect;
    requiredFeatures.sparseBinding          = supportedFeatures.sparseBinding;
    requiredFeatures.sparseResidencyImage2D = supportedFeatures.sparseResidencyImage2D;
    requiredFeatures.samplerAnisotropy      = VK_TRUE;
    requiredFeatures.tessellationShader     = VK_TRUE;
    requiredFeatures.geometryShader         = VK_TRUE;

    // Dynamic Rendering required
    VkPhysicalDeviceVulkan13Features requiredFeaturesvk13{};
    requiredFeaturesvk13.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    requiredFeaturesvk13.dynamicRendering = VK_TRUE;
    requiredFeaturesvk13.synchronization2 = VK_TRUE;

    uint32_t familyIndex = getQueueFamilyIndex(deviceContext, VK_QUEUE_GRAPHICS_BIT);
    // Queue Create Info
    float                         priority          = 1.f;
    const VkDeviceQueueCreateInfo queueCreateInfo[] = {
        { .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
          .pNext            = nullptr,
          .flags            = 0,
          .queueFamilyIndex = familyIndex,
          .queueCount       = 1,
          .pQueuePriorities = &priority },
    };

    // Device Create Info
    const VkDeviceCreateInfo deviceCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = &requiredFeaturesvk13,
        .flags                   = 0,
        .queueCreateInfoCount    = 1,
        .pQueueCreateInfos       = queueCreateInfo,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]),
        .ppEnabledExtensionNames = deviceExtensions,
        .pEnabledFeatures        = &requiredFeatures
    };

    VkDevice device = VK_NULL_HANDLE;
    VkResult result = vkCreateDevice(deviceContext.physicalDevice, &deviceCreateInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create device " << result);
    }
    return device;
}
VkQueue getQueue(DeviceContext &deviceContext, VkQueueFlags queueFlags) {
    VkQueue  queue       = VK_NULL_HANDLE;
    uint32_t familyIndex = getQueueFamilyIndex(deviceContext, queueFlags);
    vkGetDeviceQueue(deviceContext.device, familyIndex, 0, &queue);
    return queue;
}
VkSurfaceKHR createSurface(DeviceContext &deviceContext, FS::Window &windowHandle) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext     = nullptr,
        .flags     = 0,
        .hinstance = GetModuleHandleA(nullptr),
        .hwnd      = windowHandle.getNative()
    };
    VkResult result = vkCreateWin32SurfaceKHR(deviceContext.instance, &surfaceCreateInfo, nullptr, &surface);
#elif __linux__
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext  = nullptr,
        .flags  = 0,
        .dpy    = XOpenDisplay(0),
        .window = windowHandle.getNative()
    };
    VkResult result = vkCreateXlibSurfaceKHR(deviceContext.instance, &surfaceCreateInfo, nullptr, &surface);
#endif
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create surface " << result);
        return VK_NULL_HANDLE;
    }
    return surface;
}

VkCommandPool createCommandPool(DeviceContext &deviceContext, VkQueueFlags queueFlags) {
    VkCommandPool           commandPool           = VK_NULL_HANDLE;
    uint32_t                familyIndex           = getQueueFamilyIndex(deviceContext, queueFlags);
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = familyIndex
    };

    VkResult result = vkCreateCommandPool(deviceContext.device, &commandPoolCreateInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create command pool " << result);
        return VK_NULL_HANDLE;
    }
    return commandPool;
}
VkResult createCommandBuffers(DeviceContext &deviceContext, uint32_t count, VkCommandBuffer *cmdBuffers) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = deviceContext.commandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count
    };

    return vkAllocateCommandBuffers(deviceContext.device, &commandBufferAllocateInfo, cmdBuffers);
}
VkCommandBuffer startOneTimeCommandBuffer(DeviceContext &deviceContext) {
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    createCommandBuffers(deviceContext, 1, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}
void endOneTimeCommandBuffer(DeviceContext &deviceContext, VkCommandBuffer &commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = 0,
        .pWaitSemaphores      = nullptr,
        .pWaitDstStageMask    = nullptr,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores    = nullptr
    };
    vkQueueSubmit(deviceContext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(deviceContext.graphicsQueue);
    vkFreeCommandBuffers(deviceContext.device, deviceContext.commandPool, 1, &commandBuffer);
}
VkDescriptorSetLayout createDescriptorSetLayout(DeviceContext &deviceContext) {
    const VkDescriptorSetLayoutBinding descriptorBindings[2] = {
        { .binding            = 0,
          .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount    = 1,
          .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
          .pImmutableSamplers = nullptr },
        { .binding            = 1,
          .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount    = 1,
          .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
          .pImmutableSamplers = nullptr }
    };
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = 0,
        .bindingCount = sizeof(descriptorBindings) / sizeof(descriptorBindings[0]),
        .pBindings    = descriptorBindings
    };
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkResult              result              = vkCreateDescriptorSetLayout(deviceContext.device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create Descriptor Set layout " << result);
    }
    return descriptorSetLayout;
}
VkDescriptorPool createDescriptorPool(DeviceContext &deviceContext, uint32_t entityCount) {
    VkDescriptorPoolSize poolSizes[2] = {
        { .type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = MAX_FRAMES_IN_FLIGHT * entityCount },
        { .type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = MAX_FRAMES_IN_FLIGHT * entityCount }
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets       = MAX_FRAMES_IN_FLIGHT * entityCount,
        .poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]),
        .pPoolSizes    = poolSizes
    };
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkResult         result         = vkCreateDescriptorPool(deviceContext.device, &descriptorPoolCreateInfo, nullptr, &descriptorPool);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create descriptor pool " << result);
        return VK_NULL_HANDLE;
    }
    return descriptorPool;
}
VkResult createDescriptorSets(DeviceContext &deviceContext, Entity &entity) {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        layouts[i] = deviceContext.descriptorSetLayout;
    }
    VkDescriptorSetAllocateInfo allocateInfo = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = deviceContext.descriptorPool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts        = layouts
    };
    VkResult result = vkAllocateDescriptorSets(deviceContext.device, &allocateInfo, entity.descriptorSets);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate descriptor sets");
        return result;
    }

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = entity.uniformBuffer.buffer,
            .offset = 0,
            .range  = sizeof(UniformBufferData)
        };
        VkDescriptorImageInfo imageInfo = {
            .sampler     = deviceContext.sampler,
            .imageView   = entity.mesh.texture.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkWriteDescriptorSet descriptorWrites[2] = {
            { .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              .pNext            = nullptr,
              .dstSet           = entity.descriptorSets[i],
              .dstBinding       = 0,
              .dstArrayElement  = 0,
              .descriptorCount  = 1,
              .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              .pImageInfo       = nullptr,
              .pBufferInfo      = &bufferInfo,
              .pTexelBufferView = nullptr },
            { .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              .pNext            = nullptr,
              .dstSet           = entity.descriptorSets[i],
              .dstBinding       = 1,
              .dstArrayElement  = 0,
              .descriptorCount  = 1,
              .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              .pImageInfo       = &imageInfo,
              .pBufferInfo      = nullptr,
              .pTexelBufferView = nullptr }
        };

        vkUpdateDescriptorSets(deviceContext.device, (sizeof(descriptorWrites) / sizeof(descriptorWrites[0])), descriptorWrites, 0, nullptr);
    }

    return result;
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

void copyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size      = size
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}
void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height) {
    VkBufferImageCopy copyRegion = {
        .bufferOffset      = 0,
        .bufferRowLength   = 0,
        .bufferImageHeight = 0,
        .imageSubresource  = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel       = 0,
            .baseArrayLayer = 0,
            .layerCount     = 1 },
        .imageOffset = { 0, 0, 0 },
        .imageExtent = { width, height, 1 }
    };
    vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
}
void initDeviceContext(DeviceContext &deviceContext, uint32_t entityCount, FS::Window &window) {
    deviceContext.instance            = createInstance(deviceContext);
    deviceContext.physicalDevice      = getPhysicalDevice(deviceContext);
    deviceContext.device              = createDevice(deviceContext);
    deviceContext.graphicsQueue       = getQueue(deviceContext, VK_QUEUE_GRAPHICS_BIT);
    deviceContext.surface             = createSurface(deviceContext, window);
    deviceContext.commandPool         = createCommandPool(deviceContext, VK_QUEUE_GRAPHICS_BIT);
    deviceContext.descriptorPool      = createDescriptorPool(deviceContext, entityCount);
    deviceContext.descriptorSetLayout = createDescriptorSetLayout(deviceContext);
    deviceContext.sampler             = createTextureSampler(deviceContext);
}
void cleanupDeviceContext(DeviceContext &deviceContext) {
    vkDeviceWaitIdle(deviceContext.device);
    vkDestroySampler(deviceContext.device, deviceContext.sampler, nullptr);
    vkDestroyDescriptorSetLayout(deviceContext.device, deviceContext.descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(deviceContext.device, deviceContext.descriptorPool, nullptr);
    vkDestroyCommandPool(deviceContext.device, deviceContext.commandPool, nullptr);
    vkDestroySurfaceKHR(deviceContext.instance, deviceContext.surface, nullptr);
    vkDestroyDevice(deviceContext.device, nullptr);
    const auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(deviceContext.instance, "vkDestroyDebugUtilsMessengerEXT"));
    vkDestroyDebugUtilsMessengerEXT(deviceContext.instance, deviceContext.debugMessenger, nullptr);
    vkDestroyInstance(deviceContext.instance, nullptr);
}