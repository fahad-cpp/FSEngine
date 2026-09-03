#include "Vector.h"
#include "Vulkan.h" // IWYU pragma: keep
#include <FSWindow.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
constexpr uint32_t MAX_SWAPCHAIN_IMAGES = 8;
struct VulkanDeviceContext {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    uint32_t graphicsFamilyIndex;
};
struct VulkanSwapchainContext {
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
struct VulkanPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};
struct Renderer {
    uint32_t frameIndex;
    VkSemaphore renderFinishedSemaphores[MAX_SWAPCHAIN_IMAGES];
    FrameData frames[MAX_FRAMES_IN_FLIGHT];
    VulkanPipeline pipeline;
};
struct Vertex {
    Vec2 pos;
    Vec3 color;
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
VkResult createInstance(VulkanDeviceContext &deviceContext) {
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
VkResult getPhysicalDevice(VulkanDeviceContext &deviceContext) {
    // Enumerate Device
    uint32_t physicalDeviceCount = 16;
    VkPhysicalDevice physicalDevices[16];
    VkResult result = vkEnumeratePhysicalDevices(deviceContext.instance, &physicalDeviceCount, physicalDevices);
    // Select a device
    deviceContext.physicalDevice = physicalDevices[SELECTED_DEVICE];
    return result;
}
VkResult createDevice(VulkanDeviceContext &deviceContext) {

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
void getQueue(VulkanDeviceContext &deviceContext) {
    vkGetDeviceQueue(deviceContext.device, deviceContext.graphicsFamilyIndex, 0, &deviceContext.graphicsQueue);
}
VkResult createSurface(VulkanDeviceContext &deviceContext, FS::Window &windowHandle) {
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
void createSwapchain(VulkanDeviceContext &deviceContext, VulkanSwapchainContext &swapchainContext, FS::Window &window) {
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceContext.physicalDevice, deviceContext.surface, &surfaceCaps);

    // Find correct format
    constexpr uint32_t MAX_SURFACE_FORMATS = 64;
    uint32_t formatCount = MAX_SURFACE_FORMATS;
    VkSurfaceFormatKHR availableFormats[MAX_SURFACE_FORMATS];
    vkGetPhysicalDeviceSurfaceFormatsKHR(deviceContext.physicalDevice, deviceContext.surface, &formatCount, availableFormats);

    for (uint32_t i = 0; i < formatCount; ++i) {
        if (availableFormats[i].format == VK_FORMAT_R8G8B8A8_SRGB) {
            swapchainContext.surfaceFormat = availableFormats[i];
            break;
        } else if (i == (formatCount - 1)) {
            swapchainContext.surfaceFormat = availableFormats[0];
            break;
        }
    }

    // Find correct present mode
    constexpr uint32_t MAX_PRESENT_MODES = 16;
    uint32_t presentModeCount = MAX_PRESENT_MODES;
    VkPresentModeKHR presentModes[MAX_PRESENT_MODES];
    vkGetPhysicalDeviceSurfacePresentModesKHR(deviceContext.physicalDevice, deviceContext.surface, &presentModeCount, presentModes);

    VkPresentModeKHR selectedMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < presentModeCount; ++i) {
        if (presentModes[i] == selectedMode) {
            break;
        } else if (i == (presentModeCount - 1)) {
            selectedMode = presentModes[0];
            break;
        }
    }

    FS::RenderState renderState = window.getRenderState();
    uint32_t windowWidth = std::clamp<uint32_t>(renderState.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
    uint32_t windowHeight = std::clamp<uint32_t>(renderState.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
    swapchainContext.extent = (surfaceCaps.currentExtent.width != UINT_MAX) ? surfaceCaps.currentExtent : VkExtent2D{ windowWidth, windowHeight };
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = deviceContext.surface,
        .minImageCount = surfaceCaps.minImageCount + 1,
        .imageFormat = swapchainContext.surfaceFormat.format,
        .imageColorSpace = swapchainContext.surfaceFormat.colorSpace,
        .imageExtent = swapchainContext.extent,
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
    vkCreateSwapchainKHR(deviceContext.device, &swapchainCreateInfo, nullptr, &swapchainContext.swapchain);

    // Swapchain Images
    uint32_t swapchainImageCount = MAX_SWAPCHAIN_IMAGES;
    std::fill_n(swapchainContext.images, MAX_SWAPCHAIN_IMAGES, VK_NULL_HANDLE);
    vkGetSwapchainImagesKHR(deviceContext.device, swapchainContext.swapchain, &swapchainImageCount, swapchainContext.images);
    swapchainContext.imageCount = swapchainImageCount;
}
void createSwapchainImageViews(VulkanDeviceContext &deviceContext, VulkanSwapchainContext &swapchainContext) {
    VkImageSubresourceRange subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    for (uint32_t i = 0; i < swapchainContext.imageCount; ++i) {
        VkImageViewCreateInfo imageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = swapchainContext.images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchainContext.surfaceFormat.format,
            // VK_COMPONENT_SWIZZLE_IDENTITY for all components
            .components = {},
            .subresourceRange = subresourceRange
        };
        VkImageView imageView;
        vkCreateImageView(deviceContext.device, &imageViewCreateInfo, nullptr, &imageView);
        swapchainContext.imageViews[i] = imageView;
    }
}
void initSwapchainContext(VulkanDeviceContext &deviceContext, VulkanSwapchainContext &swapchainContext, FS::Window &window) {
    createSwapchain(deviceContext, swapchainContext, window);
    createSwapchainImageViews(deviceContext, swapchainContext);
}
void cleanupSwapchainContext(VulkanDeviceContext &deviceContext, VulkanSwapchainContext &swapchainContext) {
    for (uint32_t i = 0; i < swapchainContext.imageCount; ++i) {
        vkDestroyImageView(deviceContext.device, swapchainContext.imageViews[i], nullptr);
    }
    vkDestroySwapchainKHR(deviceContext.device, swapchainContext.swapchain, nullptr);
    swapchainContext.swapchain = VK_NULL_HANDLE;
}
void recreateSwapchain(VulkanDeviceContext &deviceContext, VulkanSwapchainContext &swapchainContext, FS::Window &window) {
    FS::RenderState& renderState = window.getRenderState();
    while(renderState.width == 0 || renderState.height == 0){
        window.processMessages();
    }
    vkDeviceWaitIdle(deviceContext.device);
    cleanupSwapchainContext(deviceContext, swapchainContext);
    initSwapchainContext(deviceContext, swapchainContext, window);
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

VkResult createCommandPool(VulkanDeviceContext &deviceContext) {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = deviceContext.graphicsFamilyIndex
    };

    return vkCreateCommandPool(deviceContext.device, &commandPoolCreateInfo, nullptr, &deviceContext.commandPool);
}

VkResult createCommandBuffers(VulkanDeviceContext &deviceContext, uint32_t count, VkCommandBuffer *cmdBuffers) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = deviceContext.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count
    };

    return vkAllocateCommandBuffers(deviceContext.device, &commandBufferAllocateInfo, cmdBuffers);
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
VkResult createPipelineLayout(VulkanDeviceContext &deviceContext, VulkanPipeline &pipeline) {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    return vkCreatePipelineLayout(deviceContext.device, &pipelineLayoutCreateInfo, nullptr, &pipeline.pipelineLayout);
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
void createGraphicsPipeline(VulkanDeviceContext &deviceContext, VulkanSwapchainContext &swapchainContext, VulkanPipeline &pipeline, const std::string &shaderPath) {
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceContext.physicalDevice, deviceContext.surface, &surfaceCaps);
    const std::vector<char> code = readFile(shaderPath);
    VkShaderModule module = createShaderModule(deviceContext.device, code);
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
        .pColorAttachmentFormats = &swapchainContext.surfaceFormat.format,
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
    vkCreateGraphicsPipelines(deviceContext.device, nullptr, 1, &graphicsPipelineInfo, nullptr, &pipeline.pipeline);
    vkDestroyShaderModule(deviceContext.device, module, nullptr);
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
Buffer createBuffer(VulkanDeviceContext &deviceContext, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memoryProperty) {
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
    vkCreateBuffer(deviceContext.device, &createInfo, nullptr, &buffer.buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(deviceContext.device, buffer.buffer, &memRequirements);

    uint32_t memoryIndex = getMemoryIndex(deviceContext.physicalDevice, memRequirements, memoryProperty);
    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryIndex
    };
    vkAllocateMemory(deviceContext.device, &memoryAllocateInfo, nullptr, &buffer.memory);
    vkBindBufferMemory(deviceContext.device, buffer.buffer, buffer.memory, 0);

    return buffer;
}
void copyBuffer(VulkanDeviceContext &deviceContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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
void createVertexBuffer(VulkanDeviceContext &deviceContext, const Vertex *vertices, uint32_t vertexCount, Buffer &vertexBuffer) {
    std::size_t bufferSize = sizeof(vertices[0]) * vertexCount;
    Buffer stagingBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *data = nullptr;
    vkMapMemory(deviceContext.device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, bufferSize);
    vkUnmapMemory(deviceContext.device, stagingBuffer.memory);

    vertexBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(deviceContext, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

    vkFreeMemory(deviceContext.device, stagingBuffer.memory, nullptr);
    vkDestroyBuffer(deviceContext.device, stagingBuffer.buffer, nullptr);
}
void createIndexBuffer(VulkanDeviceContext &deviceContext, const uint32_t *indices, uint32_t indexCount, Buffer &indexBuffer) {
    std::size_t bufferSize = sizeof(indices[0]) * indexCount;
    Buffer stagingBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *data = nullptr;
    vkMapMemory(deviceContext.device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memcpy(data, indices, bufferSize);
    vkUnmapMemory(deviceContext.device, stagingBuffer.memory);

    indexBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(deviceContext, stagingBuffer.buffer, indexBuffer.buffer, bufferSize);

    vkFreeMemory(deviceContext.device, stagingBuffer.memory, nullptr);
    vkDestroyBuffer(deviceContext.device, stagingBuffer.buffer, nullptr);
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
void recordCommandBuffer(FrameData frameData, VulkanSwapchainContext &swapchainContext, VulkanPipeline &pipeline, Mesh &mesh, uint32_t imageIndex) {
    VkImage image = swapchainContext.images[imageIndex];
    VkImageView imageView = swapchainContext.imageViews[imageIndex];

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
        .renderArea = { .offset = { 0, 0 }, .extent = swapchainContext.extent },
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo,
        .pDepthAttachment = nullptr,
        .pStencilAttachment = nullptr
    };
    // Rendering START
    vkCmdBeginRendering(frameData.commandBuffer, &renderingInfo);
    
    VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = static_cast<float>(swapchainContext.extent.width),
        .height = static_cast<float>(swapchainContext.extent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };
    VkRect2D scissor{
        .offset = { 0, 0 },
        .extent = swapchainContext.extent
    };
    vkCmdBindPipeline(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
    VkDeviceSize vboffset = 0;
    vkCmdBindVertexBuffers(frameData.commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &vboffset);
    vkCmdBindIndexBuffer(frameData.commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdSetViewport(frameData.commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(frameData.commandBuffer, 0, 1, &scissor);
    vkCmdDraw(frameData.commandBuffer, mesh.vertexCount, 1, 0, 0);
    vkCmdDrawIndexed(frameData.commandBuffer, mesh.indexCount, 1, 0, 0, 0);

    // Rendering END
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
void drawFrame(VulkanDeviceContext &deviceContext, VulkanSwapchainContext &swapchainContext, FS::Window &window, Renderer &renderer, Mesh &mesh) {

    FS::RenderState& renderState = window.getRenderState();
    bool windowMinimized = (renderState.width == 0) || (renderState.height == 0);
    uint32_t frameIndex = renderer.frameIndex;
    VkFence drawFence = renderer.frames[frameIndex].drawFence;
    VkSemaphore imageAcquireSemaphore = renderer.frames[frameIndex].imageAcquireSemaphore;
    vkWaitForFences(deviceContext.device, 1, &drawFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(deviceContext.device, swapchainContext.swapchain, UINT64_MAX, imageAcquireSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || windowMinimized) {
        recreateSwapchain(deviceContext, swapchainContext, window);
        return;
    }
    VkSemaphore renderFinishedSemaphore = renderer.renderFinishedSemaphores[imageIndex];

    vkResetFences(deviceContext.device, 1, &drawFence);
    recordCommandBuffer(renderer.frames[frameIndex], swapchainContext, renderer.pipeline, mesh, imageIndex);

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
    VkQueue graphicsQueue = deviceContext.graphicsQueue;
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFence);

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchainContext.swapchain,
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };
    VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        recreateSwapchain(deviceContext, swapchainContext, window);
    }
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}
void initDeviceContext(VulkanDeviceContext &deviceContext, FS::Window &window) {
    createInstance(deviceContext);
    getPhysicalDevice(deviceContext);
    createDevice(deviceContext);
    getQueue(deviceContext);
    createSurface(deviceContext, window);
    createCommandPool(deviceContext);
}
void cleanupDeviceContext(VulkanDeviceContext &deviceContext) {
    vkDeviceWaitIdle(deviceContext.device);
    vkDestroyCommandPool(deviceContext.device, deviceContext.commandPool, nullptr);
    vkDestroySurfaceKHR(deviceContext.instance, deviceContext.surface, nullptr);
    vkDestroyDevice(deviceContext.device, nullptr);
    vkDestroyInstance(deviceContext.instance, nullptr);
}
void initPipeline(VulkanDeviceContext &deviceContext, VulkanSwapchainContext &swapchainContext, VulkanPipeline &pipeline) {
    createPipelineLayout(deviceContext, pipeline);
    createGraphicsPipeline(deviceContext, swapchainContext, pipeline, "shaders/slang.spv");
}
void cleanupPipeline(VulkanDeviceContext &deviceContext, VulkanPipeline &pipeline) {
    vkDestroyPipeline(deviceContext.device, pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(deviceContext.device, pipeline.pipelineLayout, nullptr);
}
void initRenderer(VulkanDeviceContext &deviceContext, VulkanSwapchainContext &swapchainContext, Renderer &renderer) {
    createSemaphores(deviceContext.device, MAX_SWAPCHAIN_IMAGES, renderer.renderFinishedSemaphores);

    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    createCommandBuffers(deviceContext, MAX_FRAMES_IN_FLIGHT, commandBuffers);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        renderer.frames[i].commandBuffer = commandBuffers[i];
        createSemaphore(deviceContext.device, &renderer.frames[i].imageAcquireSemaphore);
        createFence(deviceContext.device, &renderer.frames[i].drawFence, VK_FENCE_CREATE_SIGNALED_BIT);
    }
    initPipeline(deviceContext, swapchainContext, renderer.pipeline);
}
void cleanupRenderer(VulkanDeviceContext &deviceContext, Renderer &renderer) {
    cleanupPipeline(deviceContext, renderer.pipeline);
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        commandBuffers[i] = renderer.frames[i].commandBuffer;
    }

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroyFence(deviceContext.device, renderer.frames[i].drawFence, nullptr);
        vkDestroySemaphore(deviceContext.device, renderer.frames[i].imageAcquireSemaphore, nullptr);
    }
    vkFreeCommandBuffers(deviceContext.device, deviceContext.commandPool, MAX_FRAMES_IN_FLIGHT, commandBuffers);
    for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i) {
        vkDestroySemaphore(deviceContext.device, renderer.renderFinishedSemaphores[i], nullptr);
    }
}
void initMesh(VulkanDeviceContext &deviceContext, Mesh &mesh, const Vertex *vertices, uint32_t vertexCount, const uint32_t *indices, uint32_t indexCount) {
    mesh.vertexCount = vertexCount;
    mesh.indexCount = indexCount;
    createVertexBuffer(deviceContext, vertices, mesh.vertexCount, mesh.vertexBuffer);
    createIndexBuffer(deviceContext, indices, mesh.indexCount, mesh.indexBuffer);
}
void cleanupMesh(VulkanDeviceContext &deviceContext, Mesh &mesh) {
    vkFreeMemory(deviceContext.device, mesh.vertexBuffer.memory, nullptr);
    vkDestroyBuffer(deviceContext.device, mesh.vertexBuffer.buffer, nullptr);

    vkFreeMemory(deviceContext.device, mesh.indexBuffer.memory, nullptr);
    vkDestroyBuffer(deviceContext.device, mesh.indexBuffer.buffer, nullptr);
}
void handleInput(FS::Window &window) {
    FS::Input &input = window.getInput();

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
}
int main() {

    const Vertex vertices[] = {
        { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
        { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
        { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
    };
    const uint32_t indices[] = {
        0, 1, 2, 2, 3, 0
    };

    FS::Window window("Vulkan Renderer", 720, 720);

    VulkanDeviceContext deviceContext = {};
    initDeviceContext(deviceContext, window);

    VulkanSwapchainContext swapchainContext = {};
    initSwapchainContext(deviceContext, swapchainContext, window);

    Renderer renderer = {};
    initRenderer(deviceContext, swapchainContext, renderer);

    uint32_t vertexCount = sizeof(vertices) / sizeof(vertices[0]);
    uint32_t indexCount = sizeof(indices) / sizeof(indices[0]);
    Mesh mesh = {};
    initMesh(deviceContext, mesh, vertices, vertexCount, indices, indexCount);

    while (window.isOpen()) {
        drawFrame(deviceContext, swapchainContext, window, renderer, mesh);
        handleInput(window);
        window.processMessages();
    }

    vkDeviceWaitIdle(deviceContext.device);
    cleanupMesh(deviceContext, mesh);
    cleanupRenderer(deviceContext, renderer);
    cleanupSwapchainContext(deviceContext, swapchainContext);
    cleanupDeviceContext(deviceContext);
    window.close();
}