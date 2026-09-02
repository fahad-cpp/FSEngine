#include "Vector.h"
#include "Vulkan.h"
#include <FSWindow.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
constexpr uint32_t MAX_SWAPCHAIN_IMAGES = 8;
struct VulkanDevice {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    uint32_t graphicsFamilyIndex;
};
struct VulkanPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};
struct VulkanSwapchain {
    VkSwapchainKHR swapchain;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D extent;
    uint32_t imageCount;
    VkImage images[MAX_SWAPCHAIN_IMAGES];
    VkImageView imageViews[MAX_SWAPCHAIN_IMAGES];
};
struct FrameData {
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAcquireSemaphore;
    VkFence drawFence;
};
struct Renderer {
    uint32_t frameIndex;
    VkSemaphore renderFinishedSemaphores[MAX_SWAPCHAIN_IMAGES];
    FrameData frames[MAX_FRAMES_IN_FLIGHT];
};
struct Vertex {
    Vec2 pos;
    Vec3 color;
};
const Vertex vertices[] = {
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
};
const uint32_t indices[] = {
    0, 1, 2, 2, 3, 0
};
struct Buffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
};
struct Mesh {
    Buffer vertexBuffer;
    Buffer indexBuffer;

    uint32_t vertexCount;
    uint32_t indexCount;
};
#define SELECTED_DEVICE 0
VkResult createInstance(VulkanDevice &device) {
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
    return vkCreateInstance(&instanceCreateInfo, nullptr, &device.instance);
}
VkResult getPhysicalDevice(VulkanDevice &device) {
    // Enumerate Device
    uint32_t physicalDeviceCount = 16;
    VkPhysicalDevice physicalDevices[16];
    VkResult result = vkEnumeratePhysicalDevices(device.instance, &physicalDeviceCount, physicalDevices);
    // Select a device
    device.physicalDevice = physicalDevices[SELECTED_DEVICE];
    return result;
}
VkResult createDevice(VulkanDevice &device) {

    const char *deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device.physicalDevice, &supportedFeatures);

    // Get supported Modern vulkan features
    VkPhysicalDeviceVulkan13Features v13features{};
    v13features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    VkPhysicalDeviceFeatures2 supportedFeatures2{};
    supportedFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    supportedFeatures2.pNext = &v13features;
    vkGetPhysicalDeviceFeatures2(device.physicalDevice, &supportedFeatures2);

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
    vkGetPhysicalDeviceQueueFamilyProperties(device.physicalDevice, &queueFamilyCount, queueFamilyProperties);
    VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
    uint32_t familyIndex = 0;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags) {
            familyIndex = i;
            break;
        }
    }
    device.graphicsFamilyIndex = familyIndex;
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

    return vkCreateDevice(device.physicalDevice, &deviceCreateInfo, nullptr, &device.device);
}
void getQueue(VulkanDevice &device) {
    vkGetDeviceQueue(device.device, device.graphicsFamilyIndex, 0, &device.graphicsQueue);
}
VkResult createSurface(VulkanDevice &device, FS::Window &windowHandle) {
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .hinstance = GetModuleHandleA(nullptr),
        .hwnd = windowHandle.getNative()
    };
    return vkCreateWin32SurfaceKHR(device.instance, &surfaceCreateInfo, nullptr, &device.surface);
#elif __linux__
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .dpy = XOpenDisplay(0),
        .window = windowHandle.getNative()
    };
    return vkCreateXlibSurfaceKHR(device.instance, &surfaceCreateInfo, nullptr, &device.surface);
#endif
}
void createSwapchain(VulkanDevice &device, VulkanSwapchain &swapchain, FS::Window &window) {
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice, device.surface, &surfaceCaps);

    // Find correct format
    constexpr uint32_t MAX_SURFACE_FORMATS = 64;
    uint32_t formatCount = MAX_SURFACE_FORMATS;
    VkSurfaceFormatKHR availableFormats[MAX_SURFACE_FORMATS];
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, device.surface, &formatCount, availableFormats);

    for (uint32_t i = 0; i < formatCount; ++i) {
        if (availableFormats[i].format == VK_FORMAT_R8G8B8A8_SRGB) {
            swapchain.surfaceFormat = availableFormats[i];
            break;
        } else if (i == (formatCount - 1)) {
            swapchain.surfaceFormat = availableFormats[0];
            break;
        }
    }

    // Find correct present mode
    constexpr uint32_t MAX_PRESENT_MODES = 16;
    uint32_t presentModeCount = MAX_PRESENT_MODES;
    VkPresentModeKHR presentModes[MAX_PRESENT_MODES];
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, device.surface, &presentModeCount, presentModes);

    VkPresentModeKHR selectedMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < presentModeCount; ++i) {
        if (presentModes[i] == selectedMode) {
            break;
        } else if (i == (presentModeCount - 1)) {
            selectedMode = presentModes[0];
            break;
        }
    }

    FS::RenderState &renderState = window.getRenderState();
    uint32_t windowWidth = std::clamp<uint32_t>(renderState.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
    uint32_t windowHeight = std::clamp<uint32_t>(renderState.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
    swapchain.extent = (surfaceCaps.currentExtent.width != UINT_MAX) ? surfaceCaps.currentExtent : VkExtent2D{ windowWidth, windowHeight };
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = device.surface,
        .minImageCount = surfaceCaps.minImageCount + 1,
        .imageFormat = swapchain.surfaceFormat.format,
        .imageColorSpace = swapchain.surfaceFormat.colorSpace,
        .imageExtent = swapchain.extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = surfaceCaps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = selectedMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    vkCreateSwapchainKHR(device.device, &swapchainCreateInfo, nullptr, &swapchain.swapchain);

    // Swapchain Images
    uint32_t swapchainImageCount = MAX_SWAPCHAIN_IMAGES;
    std::fill_n(swapchain.images, MAX_SWAPCHAIN_IMAGES, VK_NULL_HANDLE);
    vkGetSwapchainImagesKHR(device.device, swapchain.swapchain, &swapchainImageCount, swapchain.images);
    swapchain.imageCount = swapchainImageCount;
}
void createSwapchainImageViews(VulkanDevice &device, VulkanSwapchain &swapchain) {
    VkImageSubresourceRange subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    for (uint32_t i = 0; i < swapchain.imageCount; ++i) {
        VkImageViewCreateInfo imageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = swapchain.images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain.surfaceFormat.format,
            // VK_COMPONENT_SWIZZLE_IDENTITY for all components
            .components = {},
            .subresourceRange = subresourceRange
        };
        VkImageView imageView;
        vkCreateImageView(device.device, &imageViewCreateInfo, nullptr, &imageView);
        swapchain.imageViews[i] = imageView;
    }
}
void cleanupSwapchain(VulkanDevice &device, VulkanSwapchain &swapchain) {
    for (uint32_t i = 0; i < swapchain.imageCount; ++i) {
        vkDestroyImageView(device.device, swapchain.imageViews[i], nullptr);
    }
    vkDestroySwapchainKHR(device.device, swapchain.swapchain, nullptr);
    swapchain.swapchain = VK_NULL_HANDLE;
}
void recreateSwapchain(VulkanDevice &device, VulkanSwapchain &swapchain, FS::Window &window) {
    vkDeviceWaitIdle(device.device);
    cleanupSwapchain(device, swapchain);
    createSwapchain(device, swapchain, window);
    createSwapchainImageViews(device, swapchain);
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

VkResult createCommandPool(VulkanDevice &device) {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = device.graphicsFamilyIndex
    };

    return vkCreateCommandPool(device.device, &commandPoolCreateInfo, nullptr, &device.commandPool);
}

VkResult createCommandBuffers(VulkanDevice &device, uint32_t count, VkCommandBuffer *cmdBuffers) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = device.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count
    };

    return vkAllocateCommandBuffers(device.device, &commandBufferAllocateInfo, cmdBuffers);
}

// Listing 3.1
void copyBuffers(VkCommandBuffer &cmdBuffer, VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size) {
    const VkBufferCopy copyRegion = {
        .srcOffset = srcOffset,
        .dstOffset = dstOffset,
        .size = size
    };
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}
const std::vector<char> readFile(const std::string &path) {
    std::ifstream ifs(path, std::ios::ate | std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file:" << path << "\n";
        return {};
    }
    std::size_t fileSize = static_cast<std::size_t>(ifs.tellg());
    ifs.seekg(0);
    std::vector<char> fileContent(fileSize);
    ifs.read(fileContent.data(), static_cast<std::streamsize>(fileSize));
    ifs.close();
    return fileContent;
}
VkShaderModule createShaderModule(VkDevice &device, const std::vector<char> &code) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = (code.size() * sizeof(char)),
        .pCode = reinterpret_cast<const uint32_t *>(code.data())
    };
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule);
    return shaderModule;
}
VkResult createPipelineLayout(VulkanDevice &device, VulkanPipeline &pipeline) {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    return vkCreatePipelineLayout(device.device, &pipelineLayoutCreateInfo, nullptr, &pipeline.pipelineLayout);
}
VkVertexInputBindingDescription getBindingDescription() {
    return {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}
std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription() {
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
void createGraphicsPipeline(VulkanDevice &device, VulkanSwapchain &swapchain, VulkanPipeline &pipeline, const std::string &shaderPath) {
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice, device.surface, &surfaceCaps);
    const std::vector<char> code = readFile(shaderPath);
    VkShaderModule module = createShaderModule(device.device, code);
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = module,
        .pName = "vertMain",
        .pSpecializationInfo = nullptr
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = module,
        .pName = "fragMain",
        .pSpecializationInfo = nullptr
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkVertexInputBindingDescription bindingDescription = getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = getAttributeDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport{
        .x = 0.f,
        .y = 0.f,
        .width = static_cast<float>(surfaceCaps.currentExtent.width),
        .height = static_cast<float>(surfaceCaps.currentExtent.height),
        .minDepth = 0.f,
        .maxDepth = 0.f
    };

    VkRect2D scissor{
        .offset = VkOffset2D{ 0, 0 },
        .extent = surfaceCaps.currentExtent
    };
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterizerState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.f,
        .depthBiasClamp = 0.f,
        .depthBiasSlopeFactor = 0.f,
        .lineWidth = 1.f
    };

    VkPipelineMultisampleStateCreateInfo multisampleState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {}
    };

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates
    };

    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchain.surfaceFormat.format,
        .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipelineRenderingCreateInfo,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pTessellationState = nullptr,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizerState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
        .layout = pipeline.pipelineLayout,
        .renderPass = nullptr,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0
    };

    pipeline.pipeline = VK_NULL_HANDLE;
    vkCreateGraphicsPipelines(device.device, nullptr, 1, &graphicsPipelineInfo, nullptr, &pipeline.pipeline);
    vkDestroyShaderModule(device.device, module, nullptr);
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
VkResult createFences(VkDevice &device, uint32_t count, VkFence *fences, VkFenceCreateFlags flags = 0) {
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
VkResult createFence(VkDevice &device, VkFence *fence, VkFenceCreateFlags flags = 0) {
    VkFenceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags
    };
    return vkCreateFence(device, &createInfo, nullptr, fence);
}
Buffer createBuffer(VulkanDevice& device, VkBufferUsageFlags usage, VkDeviceSize size,VkMemoryPropertyFlags memoryProperty) {
    Buffer buffer = {};
    VkBufferCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };
    vkCreateBuffer(device.device, &createInfo, nullptr, &buffer.buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.device, buffer.buffer, &memRequirements);

    uint32_t memoryIndex = getMemoryIndex(device.physicalDevice, memRequirements,memoryProperty);
    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryIndex
    };
    vkAllocateMemory(device.device, &memoryAllocateInfo, nullptr, &buffer.memory);
    vkBindBufferMemory(device.device, buffer.buffer, buffer.memory, 0);

    return buffer;
}
void copyBuffer(VulkanDevice& device,VkBuffer srcBuffer,VkBuffer dstBuffer,VkDeviceSize size){
    VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = device.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device.device, &allocateInfo , &commandBuffer);
    
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
    vkQueueSubmit(device.graphicsQueue, 1, &submitInfo , VK_NULL_HANDLE);
    vkQueueWaitIdle(device.graphicsQueue);
    vkFreeCommandBuffers(device.device, device.commandPool, 1, &commandBuffer);
}
void createVertexBuffer(VulkanDevice &device, const Vertex *vertices, uint32_t vertexCount, Buffer &vertexBuffer) {
    std::size_t bufferSize = sizeof(vertices[0]) * vertexCount;
    Buffer stagingBuffer = createBuffer(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    
    void *data = nullptr;
    vkMapMemory(device.device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, bufferSize);
    vkUnmapMemory(device.device, stagingBuffer.memory);
    
    vertexBuffer = createBuffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(device, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

    vkFreeMemory(device.device, stagingBuffer.memory, nullptr);
    vkDestroyBuffer(device.device, stagingBuffer.buffer, nullptr);
}
void createIndexBuffer(VulkanDevice& device,const uint32_t* indices,uint32_t indexCount,Buffer& indexBuffer){
    std::size_t bufferSize = sizeof(indices[0]) * indexCount;
    Buffer stagingBuffer = createBuffer(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    
    void *data = nullptr;
    vkMapMemory(device.device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memcpy(data, indices, bufferSize);
    vkUnmapMemory(device.device, stagingBuffer.memory);

    indexBuffer = createBuffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(device, stagingBuffer.buffer, indexBuffer.buffer, bufferSize);

    vkFreeMemory(device.device, stagingBuffer.memory, nullptr);
    vkDestroyBuffer(device.device, stagingBuffer.buffer, nullptr);
}

void initRenderer(VulkanDevice &device, Renderer &renderer) {
    createSemaphores(device.device, MAX_SWAPCHAIN_IMAGES, renderer.renderFinishedSemaphores);

    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    createCommandBuffers(device, MAX_FRAMES_IN_FLIGHT, commandBuffers);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        renderer.frames[i].commandBuffer = commandBuffers[i];
        createSemaphore(device.device, &renderer.frames[i].imageAcquireSemaphore);
        createFence(device.device, &renderer.frames[i].drawFence, VK_FENCE_CREATE_SIGNALED_BIT);
    }
}
void cleanupRenderer(VulkanDevice& device,Renderer& renderer){
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    for(uint32_t i=0;i<MAX_FRAMES_IN_FLIGHT;++i){
        commandBuffers[i] = renderer.frames[i].commandBuffer;
    }

    for(uint32_t i=0;i<MAX_FRAMES_IN_FLIGHT;++i){
        vkDestroyFence(device.device, renderer.frames[i].drawFence, nullptr);
        vkDestroySemaphore(device.device, renderer.frames[i].imageAcquireSemaphore, nullptr);
    }
    vkFreeCommandBuffers(device.device, device.commandPool, MAX_FRAMES_IN_FLIGHT, commandBuffers);
    for(uint32_t i=0;i<MAX_SWAPCHAIN_IMAGES;++i){
        vkDestroySemaphore(device.device, renderer.renderFinishedSemaphores[i], nullptr);
    }
}
void transitionImageLayout(VkImage &image, VkCommandBuffer &commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask) {
    VkImageMemoryBarrier2 imageMemoryBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1 }
    };

    VkDependencyInfo dependencyInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        .memoryBarrierCount = 0,
        .pMemoryBarriers = nullptr,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = nullptr,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &imageMemoryBarrier
    };

    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}
void recordCommandBuffer(FrameData frameData, VulkanSwapchain &swapchain, VulkanPipeline &pipeline, Mesh &mesh, uint32_t imageIndex) {
    VkImage image = swapchain.images[imageIndex];
    VkImageView imageView = swapchain.imageViews[imageIndex];

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr
    };
    vkBeginCommandBuffer(frameData.commandBuffer, &beginInfo);

    transitionImageLayout(
        image,
        frameData.commandBuffer,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_NONE,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    VkClearValue clearColor = { VkClearColorValue{ { 0.01f, 0.02f, 0.05f, 1.f } } };
    VkRenderingAttachmentInfo attachmentInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .resolveImageView = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clearColor
    };

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .flags = 0,
        .renderArea = { .offset = { 0, 0 }, .extent = swapchain.extent },
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo,
        .pDepthAttachment = nullptr,
        .pStencilAttachment = nullptr
    };
    vkCmdBeginRendering(frameData.commandBuffer, &renderingInfo);

    // Rendering goes here
    VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = static_cast<float>(swapchain.extent.width),
        .height = static_cast<float>(swapchain.extent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };
    VkRect2D scissor{
        .offset = { 0, 0 },
        .extent = swapchain.extent
    };
    vkCmdBindPipeline(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
    VkDeviceSize vboffset = 0;
    vkCmdBindVertexBuffers(frameData.commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &vboffset);
    vkCmdBindIndexBuffer(frameData.commandBuffer,mesh.indexBuffer.buffer,0,VK_INDEX_TYPE_UINT32);
    vkCmdSetViewport(frameData.commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(frameData.commandBuffer, 0, 1, &scissor);
    vkCmdDraw(frameData.commandBuffer, mesh.vertexCount, 1, 0, 0);
    vkCmdDrawIndexed(frameData.commandBuffer, mesh.indexCount, 1, 0, 0, 0);
    vkCmdEndRendering(frameData.commandBuffer);

    transitionImageLayout(
        image,
        frameData.commandBuffer,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_2_NONE);

    vkEndCommandBuffer(frameData.commandBuffer);
}
void drawFrame(VulkanDevice &device, VulkanSwapchain &swapchain, VulkanPipeline &pipeline, FS::Window &window, Renderer &renderer, Mesh &mesh) {

    uint32_t frameIndex = renderer.frameIndex;
    VkFence drawFence = renderer.frames[frameIndex].drawFence;
    VkSemaphore imageAcquireSemaphore = renderer.frames[frameIndex].imageAcquireSemaphore;
    vkWaitForFences(device.device, 1, &drawFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(device.device, swapchain.swapchain, UINT64_MAX, imageAcquireSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(device, swapchain, window);
        return;
    }
    VkSemaphore renderFinishedSemaphore = renderer.renderFinishedSemaphores[imageIndex];

    vkResetFences(device.device, 1, &drawFence);
    recordCommandBuffer(renderer.frames[frameIndex], swapchain, pipeline, mesh, imageIndex);

    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &imageAcquireSemaphore,
        .pWaitDstStageMask = &waitDstStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &renderer.frames[frameIndex].commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderFinishedSemaphore
    };
    VkQueue graphicsQueue = device.graphicsQueue;
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFence);

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchain.swapchain,
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };
    VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        recreateSwapchain(device, swapchain, window);
    }
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}
void initDevice(VulkanDevice &device, FS::Window &window) {
    createInstance(device);
    getPhysicalDevice(device);
    createDevice(device);
    getQueue(device);
    createSurface(device, window);
    createCommandPool(device);
}
void cleanupDevice(VulkanDevice& device){
    vkDeviceWaitIdle(device.device);
    vkDestroyCommandPool(device.device, device.commandPool, nullptr);
    vkDestroySurfaceKHR(device.instance,device.surface,nullptr);
    vkDestroyDevice(device.device, nullptr);
}
void initSwapchain(VulkanDevice &device, VulkanSwapchain &swapchain, FS::Window &window) {
    createSwapchain(device, swapchain, window);
    createSwapchainImageViews(device, swapchain);
}
void initPipeline(VulkanDevice &device, VulkanSwapchain &swapchain, VulkanPipeline &pipeline) {
    createPipelineLayout(device, pipeline);
    createGraphicsPipeline(device, swapchain, pipeline, "shaders/slang.spv");
}
void cleanupPipeline(VulkanDevice& device,VulkanPipeline& pipeline){
    vkDestroyPipeline(device.device, pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(device.device, pipeline.pipelineLayout, nullptr);
}
void initMesh(VulkanDevice& device,Mesh& mesh){
    mesh.vertexCount = sizeof(vertices) / sizeof(vertices[0]);
    mesh.indexCount = sizeof(indices) / sizeof(indices[0]);
    createVertexBuffer(device, vertices, mesh.vertexCount, mesh.vertexBuffer);
    createIndexBuffer(device, indices, mesh.indexCount, mesh.indexBuffer);
}
void cleanupMesh(VulkanDevice& device,Mesh& mesh){
    vkFreeMemory(device.device, mesh.vertexBuffer.memory, nullptr);
    vkDestroyBuffer(device.device, mesh.vertexBuffer.buffer, nullptr);

    vkFreeMemory(device.device, mesh.indexBuffer.memory, nullptr);
    vkDestroyBuffer(device.device, mesh.indexBuffer.buffer, nullptr);
}
void handleInput(FS::Window &window) {
    FS::Input &input = window.getInput();

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
}
int main() {

    FS::Window window("Vulkan Renderer", 720, 720);

    VulkanDevice device = {};
    initDevice(device, window);

    VulkanSwapchain swapchain = {};
    initSwapchain(device, swapchain, window);

    Renderer renderer = {};
    initRenderer(device, renderer);

    VulkanPipeline pipeline = {};
    initPipeline(device, swapchain, pipeline);

    Mesh mesh = {};
    initMesh(device, mesh);

    while (window.isOpen()) {
        drawFrame(device, swapchain, pipeline, window, renderer, mesh);
        handleInput(window);
        window.processMessages();
    }

    vkDeviceWaitIdle(device.device);
    cleanupMesh(device,mesh);
    cleanupPipeline(device, pipeline);
    cleanupRenderer(device, renderer);
    cleanupSwapchain(device,swapchain);
    cleanupDevice(device);
    window.close();
}