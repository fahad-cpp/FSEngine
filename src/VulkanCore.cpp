#include "VulkanCore.h"
#include <cstdint>
#include <iostream>
#include <vector>
#include <vulkan/vulkan_core.h>
#define SELECTED_DEVICE 0
void checkSupportedInstanceLayers(const std::vector<const char *> &instanceLayers, std::vector<std::string> &finalLayers) {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layerProperties(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
    for (const std::string &layerName : instanceLayers) {
        bool supported = false;

        for (const VkLayerProperties &layerProperty : layerProperties) {
            if (layerProperty.layerName == layerName) {
                supported = true;
            }
        }
        if (supported) {
            finalLayers.push_back(layerName);
        } else {
            std::cout << "(WARN) Layer " << layerName << " is not supported , omitting it.\n";
        }
    }
}

void checkSupportedDeviceLayers(VkPhysicalDevice &physicalDevice, const std::vector<const char *> &deviceLayers, std::vector<std::string> &finalLayers) {
    uint32_t layerCount = 0;
    vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, nullptr);
    std::vector<VkLayerProperties> layerProperties(layerCount);
    vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, layerProperties.data());
    for (const std::string &layerName : deviceLayers) {
        bool supported = false;

        for (const VkLayerProperties &layerProperty : layerProperties) {
            if (layerProperty.layerName == layerName) {
                supported = true;
            }
        }
        if (supported) {
            finalLayers.push_back(layerName);
        } else {
            std::cout << "(WARN) Layer " << layerName << " is not supported , omitting it.\n";
        }
    }
}

static VkResult createInstance(VkInstance &instance, const std::vector<const char *> &instanceLayers, const std::vector<const char *> &instanceExtensions) {
    // check for layers support
    std::vector<std::string> finalLayers = {};
    checkSupportedInstanceLayers(instanceLayers, finalLayers);
    std::vector<const char *> finalLayersstr = {};
    for (const std::string &layer : finalLayers) {
        finalLayersstr.push_back(layer.c_str());
    }

    // Application Info
    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "MyApp",
        .applicationVersion = 1,
        .pEngineName = "FSEngine",
        .engineVersion = 1,
        .apiVersion = VK_MAKE_VERSION(1, 0, 0)
    };

    // Instance Create Info
    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = (uint32_t)finalLayers.size(),
        .ppEnabledLayerNames = finalLayersstr.data(),
        .enabledExtensionCount = (uint32_t)instanceExtensions.size(),
        .ppEnabledExtensionNames = instanceExtensions.data()
    };

    // Create the Instance
    return vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
}
VkResult getPhysicalDevice(VkInstance &instance, VkPhysicalDevice &physicalDevice) {
    // Enumerate Device
    VkResult result = VK_SUCCESS;
    uint32_t physicalDeviceCount = 0;
    std::vector<VkPhysicalDevice> physicalDevices;
    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }

    physicalDevices.resize(physicalDeviceCount);
    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
    if (result != VK_SUCCESS) {
        return result;
    }

    // Select a device
    physicalDevice = physicalDevices[SELECTED_DEVICE];
    return VK_SUCCESS;
}
std::string getMemoryPropertyFlagString(VkMemoryPropertyFlags flags) {
    std::string res = "";
    if ((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        res += "DEVICE_LOCAL ";
    }
    if ((flags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)) {
        res += "DEVICE_COHERENT_AMD ";
    }
    if ((flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)) {
        res += "HOST_CACHED ";
    }
    if ((flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        res += "HOST_COHERENT ";
    }
    if ((flags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)) {
        res += "DEVICE_UNCACHED_AMD ";
    }
    if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        res += "HOST_VISIBLE ";
    }
    if ((flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)) {
        res += "LAZILY_ALLOCATED ";
    }
    if ((flags & VK_MEMORY_PROPERTY_PROTECTED_BIT)) {
        res += "PROTECTED ";
    }
    if ((flags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV)) {
        res += "RDMA_CAPABLE_NV ";
    }

    return res;
}
std::string getMemoryHeapFlagString(VkMemoryHeapFlags flags) {
    std::string res = "";

    if (flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        res += "DEVICE_LOCAL ";
    }
    if (flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
        res += "MULTI_INSTANCE ";
    }
    if (flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR) {
        res += "MULTI_INSTANCE_KHR ";
    }

    return res;
}
std::string getQueueFlagString(VkQueueFlags flags) {
    std::string res = "";

    if (flags & VK_QUEUE_COMPUTE_BIT) {
        res += "VK_QUEUE_COMPUTE_BIT ";
    }
    if (flags & VK_QUEUE_GRAPHICS_BIT) {
        res += "VK_QUEUE_GRAPHICS_BIT ";
    }
    if (flags & VK_QUEUE_TRANSFER_BIT) {
        res += "VK_QUEUE_TRANSFER_BIT ";
    }
    if (flags & VK_QUEUE_SPARSE_BINDING_BIT) {
        res += "VK_QUEUE_SPARSE_BINDING_BIT ";
    }

    return res;
}
void getPhysicalDeviceProperties(VkPhysicalDevice &physicalDevice) {
    // PhysicalDeviceProperties
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    std::cout << "Selected Device: ";
    std::cout << properties.deviceName << "\n";

    // PhysicalDeviceMemoryProperties
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    // std::cout << "\tMemory Types:\n";
    // for(int j=0;j<memoryProperties.memoryTypeCount;j++){
    //     std::cout <<"\t\t" << "Memory Type "<<j << ":" << getMemoryPropertyFlagString(memoryProperties.memoryTypes[j].propertyFlags) << "\n";
    // }

    // std::cout << "\tHeaps:\n";
    // for(int j=0;j<memoryProperties.memoryHeapCount;j++){
    //     std::cout <<"\t\t" << "Heap "<<j<<"\n";
    //     std::cout <<"\t\t Heap Size:" << memoryProperties.memoryHeaps[j].size << " Bytes\n";
    //     std::cout <<"\t\t Heap Flags:" << getMemoryHeapFlagString(memoryProperties.memoryHeaps[j].flags) << "\n";
    // }

    // PhysicalDeviceQueueFamilyProperties
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    uint32_t qfpropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qfpropertyCount, nullptr);
    queueFamilyProperties.resize(qfpropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qfpropertyCount, queueFamilyProperties.data());

    // std::cout << "\tQueue Family Properties:\n";

    // for(int j=0;j<qfpropertyCount;j++){
    //     std::cout << "\t\tQueue Count:" << queueFamilyProperties[j].queueCount << "\n";
    //     std::cout << "\t\tTimeStamp Valid Bits:" << queueFamilyProperties[j].timestampValidBits << "\n";
    //     std::cout << "\t\tMin Image Transfer Granularity:\n"
    //     << "\t\t\tDepth:" << queueFamilyProperties[j].minImageTransferGranularity.depth << "\n"
    //     << "\t\t\tWidth:" << queueFamilyProperties[j].minImageTransferGranularity.width << "\n"
    //     << "\t\t\tHeight:" << queueFamilyProperties[j].minImageTransferGranularity.height << "\n";
    //     std::cout << "\t\tQueue Flags:" << getQueueFlagString(queueFamilyProperties[j].queueFlags) << "\n";
    // }
}
uint32_t getQueueFamilyIndex(const VkPhysicalDevice &physicalDevice) {
    uint32_t index = 0;
    uint32_t propertyCount = 0;
    std::vector<VkQueueFamilyProperties> properties;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propertyCount, nullptr);
    properties.resize(propertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propertyCount, properties.data());

    for (uint32_t i = 0; i < propertyCount; i++) {
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            index = i;
            break;
        }
    }

    return index;
}
VkResult createDevice(VkPhysicalDevice &physicalDevice, VkDevice &device, const std::vector<const char *> &deviceLayers, const std::vector<const char *> &deviceExtensions) {

    std::vector<std::string> finalLayers = {};
    checkSupportedDeviceLayers(physicalDevice, deviceLayers, finalLayers);
    std::vector<const char *> finalLayersstr = {};
    for (const std::string &layer : finalLayers) {
        finalLayersstr.push_back(layer.c_str());
    }

    uint32_t familyIndex = getQueueFamilyIndex(physicalDevice);

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    // Specify required Features
    // An Example where tesselation shader and geometry shaders are must have
    // and multiDrawIndirect is supported if the device supports it
    VkPhysicalDeviceFeatures requiredFeatures = {};
    requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
    requiredFeatures.sparseBinding = supportedFeatures.sparseBinding;
    requiredFeatures.sparseResidencyImage2D = supportedFeatures.sparseResidencyImage2D;
    requiredFeatures.tessellationShader = VK_TRUE;
    requiredFeatures.geometryShader = VK_TRUE;

    // Queue Create Info
    float priority = 1.f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = familyIndex,
        .queueCount = 1,
        .pQueuePriorities = &priority
    };

    // Device Create Info
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledLayerCount = (uint32_t)finalLayers.size(),
        .ppEnabledLayerNames = finalLayersstr.data(),
        .enabledExtensionCount = (uint32_t)deviceExtensions.size(),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &requiredFeatures
    };

    return vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
}

void printInstanceLayersAndExt() {
    uint32_t propertyCount = 0;
    std::vector<VkLayerProperties> layerProperties = {};
    vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);

    layerProperties.resize(propertyCount);
    vkEnumerateInstanceLayerProperties(&propertyCount, layerProperties.data());
    std::cout << "Instance Layers:" << propertyCount << "\n";
    for (int i = 0; i < propertyCount; i++) {
        std::cout << layerProperties[i].layerName << "\n";
    }

    propertyCount = 0;
    std::vector<VkExtensionProperties> extensionProperties = {};
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);

    extensionProperties.resize(propertyCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, extensionProperties.data());
    std::cout << "Instance Extensions:" << propertyCount << "\n";
    for (int i = 0; i < propertyCount; i++) {
        std::cout << extensionProperties[i].extensionName << "\n";
    }
}
void printDeviceLayersAndExt(VkPhysicalDevice &physicalDevice) {
    uint32_t propertyCount = 0;
    std::vector<VkLayerProperties> layerProperties = {};
    vkEnumerateDeviceLayerProperties(physicalDevice, &propertyCount, nullptr);

    layerProperties.resize(propertyCount);
    vkEnumerateDeviceLayerProperties(physicalDevice, &propertyCount, layerProperties.data());
    std::cout << "Device Layers:" << propertyCount << "\n";
    for (int i = 0; i < propertyCount; i++) {
        std::cout << layerProperties[i].layerName << "\n";
    }

    propertyCount = 0;
    std::vector<VkExtensionProperties> extensionProperties = {};
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr);

    extensionProperties.resize(propertyCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, extensionProperties.data());

    std::cout << "Device Extensions:" << propertyCount << "\n";
    for (int i = 0; i < propertyCount; i++) {
        std::cout << extensionProperties[i].extensionName << "\n";
    }
}

#ifdef _WIN32
VkResult createSurface(const VkInstance &instance, FS::Window &windowHandle, VkSurfaceKHR &surface) {
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .hinstance = GetModuleHandleA(nullptr),
        .hwnd = windowHandle.getNative()
    };
    return vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
}
#elif __linux__
VkResult createSurface(const VkInstance &instance, FS::Window &windowHandle, VkSurfaceKHR &surface) {
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .dpy = XOpenDisplay(0),
        .window = windowHandle.getNative()
    };
    return vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
}
#endif
VkResult createSwapchain(const VkPhysicalDevice &physicalDevice, const VkDevice &device, const VkSurfaceKHR &surface, VkSwapchainKHR &swapchain) {
    uint32_t formatCount;
    std::vector<VkSurfaceFormatKHR> formats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
    VkSurfaceFormatKHR chosenFormat = formats[0];

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .surface = surface,
        .minImageCount = (caps.minImageCount + 1),
        .imageFormat = chosenFormat.format,
        .imageColorSpace = chosenFormat.colorSpace,
        .imageExtent = caps.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };
    return vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
}

uint32_t getMemoryIndex(VkPhysicalDevice &physicalDevice, VkMemoryRequirements requirements, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags) {
    uint32_t selectedType = ~0u;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; ++memoryType) {
        if (requirements.memoryTypeBits & (1 << memoryType)) {
            const VkMemoryType &type = memoryProperties.memoryTypes[memoryType];
            if ((type.propertyFlags & preferredFlags) == preferredFlags) {
                selectedType = memoryType;
                break;
            }
        }
    }
    if (selectedType == ~0u) {
        for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; ++memoryType) {
            if (requirements.memoryTypeBits & (1 << memoryType)) {
                const VkMemoryType &type = memoryProperties.memoryTypes[memoryType];
                if ((type.propertyFlags & requiredFlags) == requiredFlags) {
                }
            }
        }
    }

    return selectedType;
}

// Resources creation
VkResult createBuffer(VkDevice &device, VkBuffer &buffer, VkPhysicalDevice &physicalDevice, VkDeviceMemory &memory) {
    static const VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = 1024 * 1024,
        .usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };

    VkResult result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
    if (result != VK_SUCCESS) {
        return result;
    }
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    uint32_t memoryIndex = getMemoryIndex(physicalDevice, memoryRequirements, 0, 0);

    VkMemoryAllocateInfo memoryAllocateInfo;
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryIndex;
    vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory);
    vkBindBufferMemory(device, buffer, memory, 0);

    return result;
}

VkResult createBufferView(VkDevice &device, VkBuffer &buffer, VkBufferView &bufferView) {
    static const VkBufferViewCreateInfo bufferViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .buffer = buffer,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .offset = 0,
        .range = 1024 * 1024
    };

    return vkCreateBufferView(device, &bufferViewCreateInfo, nullptr, &bufferView);
}

VkResult createImageView(VkDevice &device, VkImage &image, VkImageView &imageView) {
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;

    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageViewCreateInfo.components = {};
    imageViewCreateInfo.subresourceRange = subresourceRange;

    return vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView);
}

VkResult createCubemap(VkPhysicalDevice &physicalDevice, VkDevice &device, VkImage &cubemap, VkDeviceMemory &memory) {
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent = VkExtent3D{ 1080, 1080, 1 }; // must be square for
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 6; // cube faces
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = nullptr;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkResult result = vkCreateImage(device, &imageCreateInfo, nullptr, &cubemap);
    if (result != VK_SUCCESS) {
        return result;
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, cubemap, &memoryRequirements);

    uint32_t memoryIndex = getMemoryIndex(physicalDevice, memoryRequirements, 0, 0);

    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = memoryIndex;

    vkAllocateMemory(device, &allocateInfo, nullptr, &memory);
    vkBindImageMemory(device, cubemap, memory, 0);
    return result;
}

VkResult createCubemapView(VkDevice &device, VkImage &cubemap, VkImageView &cubemapView) {
    VkImageSubresourceRange arraySubResRange;
    arraySubResRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    arraySubResRange.baseArrayLayer = 0;
    arraySubResRange.layerCount = 6;
    arraySubResRange.baseMipLevel = 0;
    arraySubResRange.levelCount = 1;

    // Create Image Array View
    VkImageViewCreateInfo arrayImageViewCreateInfo = {};
    arrayImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    arrayImageViewCreateInfo.pNext = nullptr;
    arrayImageViewCreateInfo.flags = 0;
    arrayImageViewCreateInfo.image = cubemap;
    arrayImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    arrayImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    arrayImageViewCreateInfo.components = {};
    arrayImageViewCreateInfo.subresourceRange = arraySubResRange;

    return vkCreateImageView(device, &arrayImageViewCreateInfo, nullptr, &cubemapView);
}

VkResult createImage(VkPhysicalDevice &physicalDevice, VkDevice &device, VkImage &image, VkDeviceMemory &memory) {
    VkFormat selectedFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Optimal Image
    //  static const VkImageCreateInfo imageCreateInfo = {
    //      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    //      .pNext = nullptr,
    //      .flags = 0,
    //      .imageType = VK_IMAGE_TYPE_2D,
    //      .format = selectedFormat,
    //      .extent = VkExtent3D{1920,1080,1},
    //      .mipLevels = 10,
    //      .arrayLayers = 1,
    //      .samples = VK_SAMPLE_COUNT_1_BIT,
    //      .tiling = VK_IMAGE_TILING_OPTIMAL,
    //      .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
    //      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    //      .queueFamilyIndexCount = 0,
    //      .pQueueFamilyIndices = nullptr,
    //      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    //  };

    // Linear Image
    static const VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = selectedFormat,
        .extent = VkExtent3D{ 1920, 1080, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };
    VkResult res = vkCreateImage(device, &imageCreateInfo, nullptr, &image);
    if (res != VK_SUCCESS) {
        return res;
    }

    const VkImageSubresource subresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .arrayLayer = 0
    };
    VkSubresourceLayout subResLayout;
    vkGetImageSubresourceLayout(device, image, &subresource, &subResLayout);

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);

    uint32_t memoryIndex = getMemoryIndex(physicalDevice, memoryRequirements, 0, 0);

    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = memoryIndex;

    vkAllocateMemory(device, &allocateInfo, nullptr, &memory);

    vkBindImageMemory(device, image, memory, 0);

    return res;
}

VkResult createSparseImage(VkPhysicalDevice &physicalDevice, VkDevice &device, VkImage &image, VkDeviceMemory &memory) {
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    if (features.sparseBinding == VK_FALSE || features.sparseResidencyImage2D == VK_FALSE) {
        std::cerr << "Sparse Images are not supported on your device\n";
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = VkExtent3D{ 1024, 1024, 1 },
        .mipLevels = 10,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };
    VkResult result = vkCreateImage(device, &imageCreateInfo, nullptr, &image);
    if (result != VK_SUCCESS) {
        return result;
    }

    uint32_t requirementCount = 0;
    vkGetImageSparseMemoryRequirements(device, image, &requirementCount, nullptr);
    std::vector<VkSparseImageMemoryRequirements> memoryRequirements(requirementCount);
    vkGetImageSparseMemoryRequirements(device, image, &requirementCount, memoryRequirements.data());

    for (const VkSparseImageMemoryRequirements &req : memoryRequirements) {
        const VkExtent3D &granularity = req.formatProperties.imageGranularity;
        std::cout << "\tImage Granularity:" << granularity.width << "x" << granularity.height << "x" << granularity.depth << "\n";
        std::cout << "\tAspect Mask:" << ((req.formatProperties.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)?"Color ":"") << ((req.formatProperties.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)?"Depth ":"") << ((req.formatProperties.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)?"Stencil ":"") << "\n";
        std::cout << "\tFlags:" << ((req.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT) ? "Single Miptail " : "") << ((req.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT) ? "Aligned Mip Size " : "") << ((req.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT) ? "Non Standard Block Size" : "") << "\n";
        std::cout << "\tMipTailSize:" << req.imageMipTailSize << "\n";
        std::cout << "\tMipTailFirstLOD:" << req.imageMipTailFirstLod << "\n";
        std::cout << "\tMipTailOffset:" << req.imageMipTailOffset << "\n";
        std::cout << "\tMipTailStride:" << req.imageMipTailStride << "\n";
    }
    return result;
}

int VulkanCore::init() {
    // Create Instance
    VkResult result = createInstance(m_instance, instanceLayers, instanceExtensions);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Instance:" << result << "\n";
        std::cerr << result;
        return 1;
    } else {
        std::cout << "Successfully created Instance\n";
    }

    // Get Physical Device
    if (getPhysicalDevice(m_instance, m_physicalDevice) != VK_SUCCESS) {
        std::cerr << "Failed to query Physical Device\n";
        return 1;
    } else {
        std::cout << "Successfully Received Physical Device\n";
    }

    getPhysicalDeviceProperties(m_physicalDevice);

    // Create Device
    if ((result = createDevice(m_physicalDevice, m_device, deviceLayers, deviceExtensions)) != VK_SUCCESS) {
        std::cerr << "Failed to create Logical Device:" << result << "\n";
        return 1;
    } else {
        std::cout << "Successfully created Logical Device\n";
    }

    // printInstanceLayersAndExt();
    // printDeviceLayersAndExt(m_physicalDevice);

    // Create Buffer
    VkDeviceMemory bufferMemory;
    if ((result = createBuffer(m_device, m_buffer, m_physicalDevice, bufferMemory)) != VK_SUCCESS) {
        std::cerr << "Failed to create buffer:" << result << "\n";
        return 1;
    } else {
        std::cout << "Successfully created buffer\n";
    }
    m_memory.push_back(bufferMemory);

    // Create Window
    m_window = new FS::Window("Vulkan Window", 720, 720);
    if (!m_window->isOpen()) {
        std::cerr << "Failed to create window\n";
    } else {
        std::cout << "Successfully created window\n";
    }

    // Create Window Surface
    if ((result = createSurface(m_instance, *m_window, m_surface)) != VK_SUCCESS) {
        std::cerr << "Failed to create Surface:" << result << "\n";
        return 1;
    } else {
        std::cout << "Successfully created surface\n";
    }

    // Create Swapchain
    if ((result = createSwapchain(m_physicalDevice, m_device, m_surface, m_swapchain)) != VK_SUCCESS) {
        std::cerr << "Failed to create Swapchain:" << result << "\n";
        return 1;
    } else {
        std::cout << "Successfully created Swapchain\n";
    }

    // Create Image
    VkImage image;
    VkDeviceMemory imageMemory;
    result = createImage(m_physicalDevice, m_device, image, imageMemory);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Image:" << result << "\n";
        return 1;
    } else {
        std::cout << "Successfully created Image\n";
    }
    m_images.push_back(image);
    m_memory.push_back(imageMemory);

    // Query Compressed formats support
    VkPhysicalDeviceFeatures feat;
    vkGetPhysicalDeviceFeatures(m_physicalDevice, &feat);
    std::cout << ((feat.textureCompressionBC) ? "BC texture compression supported\n" : "BC texture compression not supported\n");
    std::cout << ((feat.textureCompressionETC2) ? "ETC2 texture compression supported\n" : "ETC2 texture compression not supported\n");
    std::cout << ((feat.textureCompressionASTC_LDR) ? "ASTC texture compression supported\n" : "ASTC texture compression not supported\n");

    // Create buffer view
    VkBufferView bufferView;
    result = createBufferView(m_device, m_buffer, bufferView);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Buffer View\n";
    } else {
        std::cout << "Successfully created buffer view\n";
    }
    // Destroy buffer view
    vkDestroyBufferView(m_device, bufferView, nullptr);
    std::cout << "Destoyed Buffer View\n";

    // Create Image View
    VkImageView imageView;
    result = createImageView(m_device, m_images[0], imageView);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Image View\n";
    } else {
        std::cout << "Successfully created Image View\n";
    }
    m_imageViews.push_back(imageView);

    // Create Cubemap
    VkImage cubemap;
    VkDeviceMemory cubemapMemory;
    result = createCubemap(m_physicalDevice, m_device, cubemap, cubemapMemory);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create cubemap image object\n";
    } else {
        std::cout << "Successfully created cubemap image object\n";
    }
    m_images.push_back(cubemap);
    m_memory.push_back(cubemapMemory);

    // Create Cubemap view
    VkImageView cubemapView;
    result = createCubemapView(m_device, cubemap, cubemapView);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create cubemap image view\n";
    } else {
        std::cout << "Successfully created cubemap image view\n";
    }
    m_imageViews.push_back(cubemapView);

    // Create sparse image
    VkImage sparseImage;
    VkDeviceMemory sparseImageMemory;
    result = createSparseImage(m_physicalDevice, m_device, sparseImage, sparseImageMemory);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create sparse image\n";
    } else {
        std::cout << "Successfully created sparse image\n";
    }
    m_images.push_back(sparseImage);
    // m_memory.push_back(sparseImageMemory);

    return VK_SUCCESS;
}

void VulkanCore::cleanup() {
    for (VkImageView &imageView : m_imageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    std::cout << "Destroyed Image Views\n";

    for (VkImage &image : m_images) {
        vkDestroyImage(m_device, image, nullptr);
    }
    m_images.clear();
    std::cout << "Destroyed Images\n";

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    std::cout << "Destroyed Swapchain\n";

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    std::cout << "Destroyed Surface\n";

    delete m_window;
    std::cout << "Destroyed Window\n";

    vkDestroyBuffer(m_device, m_buffer, nullptr);
    std::cout << "Destroyed Buffer\n";

    for (VkDeviceMemory &memory : m_memory) {
        vkFreeMemory(m_device, memory, nullptr);
    }

    vkDeviceWaitIdle(m_device);
    vkDestroyDevice(m_device, nullptr);
    std::cout << "Destroyed Device\n";

    vkDestroyInstance(m_instance, nullptr);
    std::cout << "Destroyed Instance\n";
}
