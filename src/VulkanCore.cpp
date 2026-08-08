#include "VulkanCore.h"
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <vector>
#include <vulkan/vulkan_core.h>
#define SELECTED_DEVICE 0
void checkSupportedInstanceLayers(const std::vector<const char *> &instanceLayers, std::vector<std::string> &finalLayers) {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layerProperties(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
    for (const std::string layerName : instanceLayers) {
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

static VkResult createInstance(VkInstance &instance, const std::vector<const char *> &layers, const std::vector<const char *> &instanceExtensions) {
    // check for layers support
    std::vector<std::string> finalLayers = {};
    checkSupportedInstanceLayers(layers, finalLayers);
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
        .apiVersion = VK_MAKE_API_VERSION(0, 1, 4, 0)
    };

    // Instance Create Info
    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
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
    std::cout << "\tMemory Types:\n";
    for (uint32_t j = 0; j < memoryProperties.memoryTypeCount; ++j) {
        std::cout << "\t\t" << "Memory Type " << j << ":" << getMemoryPropertyFlagString(memoryProperties.memoryTypes[j].propertyFlags) << "\n";
    }

    std::cout << "\tHeaps:\n";
    for (uint32_t j = 0; j < memoryProperties.memoryHeapCount; ++j) {
        std::cout << "\t\t" << "Heap " << j << "\n";
        std::cout << "\t\t Heap Size:" << memoryProperties.memoryHeaps[j].size << " Bytes\n";
        std::cout << "\t\t Heap Flags:" << getMemoryHeapFlagString(memoryProperties.memoryHeaps[j].flags) << "\n";
    }

    // PhysicalDeviceQueueFamilyProperties
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    uint32_t qfpropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qfpropertyCount, nullptr);
    queueFamilyProperties.resize(qfpropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qfpropertyCount, queueFamilyProperties.data());

    std::cout << "\tQueue Family Properties:\n";

    for (uint32_t j = 0; j < qfpropertyCount; ++j) {
        std::cout << "\t\tQueue Count:" << queueFamilyProperties[j].queueCount << "\n";
        std::cout << "\t\tTimeStamp Valid Bits:" << queueFamilyProperties[j].timestampValidBits << "\n";
        std::cout << "\t\tMin Image Transfer Granularity:\n"
                  << "\t\t\tDepth:" << queueFamilyProperties[j].minImageTransferGranularity.depth << "\n"
                  << "\t\t\tWidth:" << queueFamilyProperties[j].minImageTransferGranularity.width << "\n"
                  << "\t\t\tHeight:" << queueFamilyProperties[j].minImageTransferGranularity.height << "\n";
        std::cout << "\t\tQueue Flags:" << getQueueFlagString(queueFamilyProperties[j].queueFlags) << "\n";
    }
}
uint32_t getQueueFamilyIndex(const VkPhysicalDevice &physicalDevice, const VkQueueFlags &flags) {
    uint32_t index = 0;
    uint32_t propertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> properties(propertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propertyCount, properties.data());

    for (uint32_t i = 0; i < propertyCount; i++) {
        if ((properties[i].queueFlags & flags) == flags) {
            return i;
        }
    }

    std::cerr << "(WARN!):No Queue Family Found for specified flags\n";
    return index;
}
VkResult createDevice(VkPhysicalDevice &physicalDevice, VkDevice &device, const std::vector<const char *> &deviceExtensions) {

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

    uint32_t familyIndex = getQueueFamilyIndex(physicalDevice, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_TRANSFER_BIT);
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
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = queueCreateInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
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
    for (uint32_t i = 0; i < propertyCount; ++i) {
        std::cout << layerProperties[i].layerName << "\n";
    }

    propertyCount = 0;
    std::vector<VkExtensionProperties> extensionProperties = {};
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);

    extensionProperties.resize(propertyCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, extensionProperties.data());
    std::cout << "Instance Extensions:" << propertyCount << "\n";
    for (uint32_t i = 0; i < propertyCount; ++i) {
        std::cout << extensionProperties[i].extensionName << "\n";
    }
}
void printDeviceExt(VkPhysicalDevice &physicalDevice) {
    uint32_t propertyCount = 0;
    std::vector<VkExtensionProperties> extensionProperties = {};
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr);

    extensionProperties.resize(propertyCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, extensionProperties.data());

    std::cout << "Device Extensions:" << propertyCount << "\n";
    for (uint32_t i = 0; i < propertyCount; ++i) {
        std::cout << extensionProperties[i].extensionName << "\n";
    }
}

#ifdef _WIN32
VkResult createSurface(const VkInstance &instance, FS::Window &windowHandle, VkSurfaceKHR &surface) {
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
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
VkResult createSwapchain(VkPhysicalDevice &physicalDevice, VkDevice &device, VkSurfaceKHR &surface, VkSwapchainKHR &swapchain, FS::Window &window) {
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps);

    // Find correct format
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, availableFormats.data());
    VkSurfaceFormatKHR selectedFormat;
    for (uint32_t i = 0; i < availableFormats.size(); ++i) {
        if (availableFormats[i].format == VK_FORMAT_R8G8B8A8_SRGB) {
            selectedFormat = availableFormats[i];
            break;
        } else if (i == (availableFormats.size() - 1)) {
            selectedFormat = availableFormats[0];
        }
    }

    // Find correct present mode
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
    VkPresentModeKHR selectedMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < presentModes.size(); ++i) {
        if (presentModes[i] == selectedMode) {
            break;
        } else if (i == (presentModes.size() - 1)) {
            selectedMode = presentModes[0];
        }
    }

    FS::RenderState &renderState = window.getRenderState();
    uint32_t windowWidth = std::clamp<uint32_t>(renderState.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
    uint32_t windowHeight = std::clamp<uint32_t>(renderState.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
    VkExtent2D extent = (surfaceCaps.currentExtent.width != UINT_MAX) ? surfaceCaps.currentExtent : VkExtent2D{ windowWidth, windowHeight };
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface,
        .minImageCount = surfaceCaps.minImageCount + 1,
        .imageFormat = selectedFormat.format,
        .imageColorSpace = selectedFormat.colorSpace,
        .imageExtent = extent,
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
    return vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
}

uint32_t getMemoryIndex(VkPhysicalDevice &physicalDevice, VkMemoryRequirements requirements, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags) {
    uint32_t selectedType = ~0u;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    // Check for preferred flags first
    for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; ++memoryType) {
        if (requirements.memoryTypeBits & (1 << memoryType)) {
            const VkMemoryType &type = memoryProperties.memoryTypes[memoryType];
            if ((type.propertyFlags & preferredFlags) == preferredFlags) {
                selectedType = memoryType;
                break;
            }
        }
    }
    // check for required flags if preferred flags do not match
    if (selectedType == ~0u) {
        for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; ++memoryType) {
            if (requirements.memoryTypeBits & (1 << memoryType)) {
                const VkMemoryType &type = memoryProperties.memoryTypes[memoryType];
                if ((type.propertyFlags & requiredFlags) == requiredFlags) {
                    selectedType = memoryType;
                }
            }
        }
    }

    return selectedType;
}

// Resources creation
VkResult createBuffer(VkDevice &device, VkBuffer &buffer, VkPhysicalDevice &physicalDevice, VkDeviceMemory &memory, const VkBufferUsageFlags &usage, const VkDeviceSize size) {
    const VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
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

    VkPhysicalDeviceProperties prop;
    vkGetPhysicalDeviceProperties(physicalDevice, &prop);
    uint32_t arrayLayers = std::min(6u, prop.limits.maxImageArrayLayers);
    VkExtent3D extent = VkExtent3D{ 1024, 1024, 1 };
    uint32_t maxDimension = std::max(std::max(extent.width, extent.height), extent.depth);
    uint32_t maxMipLevel = static_cast<uint32_t>(std::bit_width(maxDimension));
    if (arrayLayers != 6u) {
        std::cout << "(WARN):arrayLayers reduced to " << arrayLayers << "\n";
    }
    uint32_t mipLevels = std::min(10u, maxMipLevel);
    if (mipLevels != 10u) {
        std::cout << "(WARN):mipLevels reduced to " << mipLevels << "\n";
    }

    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = extent,
        .mipLevels = mipLevels,
        .arrayLayers = arrayLayers,
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

    uint32_t propertyCount = 0;
    vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, &propertyCount, nullptr);
    std::vector<VkSparseImageFormatProperties> formatProperties(propertyCount);
    vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, &propertyCount, formatProperties.data());

    VkMemoryRequirements imageMemoryRequirements = {};
    vkGetImageMemoryRequirements(device, image, &imageMemoryRequirements);

    uint32_t memoryTypeIndex = getMemoryIndex(physicalDevice, imageMemoryRequirements, 0, 0);
    VkMemoryAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = imageMemoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };
    vkAllocateMemory(device, &allocateInfo, nullptr, &memory);

    VkSparseMemoryBind sparseMemoryBind = {
        .resourceOffset = 0,
        .size = imageMemoryRequirements.size,
        .memory = memory,
        .memoryOffset = 0,
        .flags = 0
    };

    VkSparseImageOpaqueMemoryBindInfo imageOpaqueBindInfo = {
        .image = image,
        .bindCount = 1,
        .pBinds = &sparseMemoryBind
    };

    VkBindSparseInfo bindInfo = {
        .sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .bufferBindCount = 0,
        .pBufferBinds = nullptr,
        .imageOpaqueBindCount = 1,
        .pImageOpaqueBinds = &imageOpaqueBindInfo,
        .imageBindCount = 0,
        .pImageBinds = nullptr,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr
    };

    VkQueue queue;
    uint32_t familyIndex = getQueueFamilyIndex(physicalDevice, VK_QUEUE_SPARSE_BINDING_BIT);
    vkGetDeviceQueue(device, familyIndex, 0, &queue);
    vkQueueBindSparse(queue, 1, &bindInfo, VK_NULL_HANDLE);
    return result;
}
VkResult createCommandPool(VkDevice &device, VkPhysicalDevice &physicalDevice, VkCommandPool &commandPool) {
    uint32_t queueGraphicsFamilyIndex = getQueueFamilyIndex(physicalDevice, VK_QUEUE_TRANSFER_BIT);

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueGraphicsFamilyIndex
    };

    return vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
}

VkResult createCommandBuffer(VkDevice &device, VkCommandPool &commandPool, VkCommandBuffer &cmdBuffer) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    return vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &cmdBuffer);
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
    VkResult res = vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule);
    if (res != VK_SUCCESS) {
        std::cerr << "Failed to create Shader Module\n";
        return VK_NULL_HANDLE;
    } else {
        std::cout << "Successfully created Shader Module\n";
    }
    return shaderModule;
}
int VulkanCore::init() {
    // Create Instance
    VkResult result = createInstance(m_instance, layers, instanceExtensions);
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
    if ((result = createDevice(m_physicalDevice, m_device, deviceExtensions)) != VK_SUCCESS) {
        std::cerr << "Failed to create Logical Device:" << result << "\n";
        return 1;
    } else {
        std::cout << "Successfully created Logical Device\n";
    }

    // printInstanceLayersAndExt();
    // printDeviceExt(m_physicalDevice);

    // Create Buffer
    VkDeviceMemory bufferMemory;
    if ((result = createBuffer(m_device, m_buffer, m_physicalDevice, bufferMemory, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 1024 * 1024)) != VK_SUCCESS) {
        std::cerr << "Failed to create buffer:" << result << "\n";
        return 1;
    } else {
        std::cout << "Successfully created buffer\n";
    }
    m_memory.push_back(bufferMemory);

    // Create Window
    window = new FS::Window("Vulkan Window", 720, 720);
    if (!window->isOpen()) {
        std::cerr << "Failed to create window\n";
        return 1;
    } else {
        std::cout << "Successfully created window\n";
    }

    // Create Window Surface
    if ((result = createSurface(m_instance, *window, m_surface)) != VK_SUCCESS) {
        std::cerr << "Failed to create Surface:" << result << "\n";
        return 1;
    } else {
        std::cout << "Successfully created surface\n";
    }

    // Create Swapchain
    if ((result = createSwapchain(m_physicalDevice, m_device, m_surface, m_swapchain, *window)) != VK_SUCCESS) {
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
    // std::cout << ((feat.textureCompressionBC) ? "BC texture compression supported\n" : "BC texture compression not supported\n");
    // std::cout << ((feat.textureCompressionETC2) ? "ETC2 texture compression supported\n" : "ETC2 texture compression not supported\n");
    // std::cout << ((feat.textureCompressionASTC_LDR) ? "ASTC texture compression supported\n" : "ASTC texture compression not supported\n");

    // Create buffer view
    result = createBufferView(m_device, m_buffer, m_bufferView);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Buffer View:" << result << "\n";
        return 1;
    } else {
        std::cout << "Successfully created Buffer View\n";
    }

    // Create Image View
    VkImageView imageView;
    result = createImageView(m_device, m_images[0], imageView);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Image View\n";
    } else {
        std::cout << "Successfully created Image View\n";
        m_imageViews.push_back(imageView);
    }

    // Create Cubemap
    VkImage cubemap;
    VkDeviceMemory cubemapMemory;
    result = createCubemap(m_physicalDevice, m_device, cubemap, cubemapMemory);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create cubemap image object:" << result << "\n";
    } else {
        std::cout << "Successfully created cubemap image object\n";
        m_images.push_back(cubemap);
        m_memory.push_back(cubemapMemory);
    }

    // Create Cubemap view
    VkImageView cubemapView;
    result = createCubemapView(m_device, cubemap, cubemapView);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create cubemap image view:" << result << "\n";
    } else {
        std::cout << "Successfully created cubemap image view\n";
        m_imageViews.push_back(cubemapView);
    }

    // Create sparse image
    VkImage sparseImage;
    VkDeviceMemory sparseImageMemory;
    result = createSparseImage(m_physicalDevice, m_device, sparseImage, sparseImageMemory);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create sparse image\n";
    } else {
        std::cout << "Successfully created sparse image\n";
        m_images.push_back(sparseImage);
        m_memory.push_back(sparseImageMemory);
    }

    // Create a command pool
    result = createCommandPool(m_device, m_physicalDevice, m_commandPool);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create command pool:" << result << "\n";
    } else {
        std::cout << "Successfully created command pool\n";
    }

    // Create a command buffer
    result = createCommandBuffer(m_device, m_commandPool, m_commandBuffer);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create command buffer:" << result << "\n";
    } else {
        std::cout << "Successfully created command buffer\n";
    }

    VkBuffer dstBuffer;
    VkDeviceMemory dstBufferMemory;
    createBuffer(m_device, dstBuffer, m_physicalDevice, dstBufferMemory, VK_BUFFER_USAGE_TRANSFER_DST_BIT, 1024 * 1024);
    m_memory.push_back(dstBufferMemory);

    // Begin Recording Commands
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr
    };
    vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

    copyBuffers(m_commandBuffer, m_buffer, dstBuffer, 0, 0, 1024 * 1024);

    // End Recording Commands
    vkEndCommandBuffer(m_commandBuffer);

    VkQueue queue;
    uint32_t familyIndex = getQueueFamilyIndex(m_physicalDevice, VK_QUEUE_TRANSFER_BIT);
    vkGetDeviceQueue(m_device, familyIndex, 0, &queue);
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr
    };
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkDestroyBuffer(m_device, dstBuffer, nullptr);

    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, nullptr);
    m_swapchainImages.resize(swapchainImageCount);
    VkResult res = vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, m_swapchainImages.data());
    if (res != VK_SUCCESS) {
        std::cerr << "Failed to get swapchain images\n";
    } else {
        std::cout << "Successfully got swapchain images\n";
    }

    VkImageSubresourceRange subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    for (VkImage &swapchainImage : m_swapchainImages) {
        VkImageViewCreateInfo imageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = swapchainImage,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .components = {},
            .subresourceRange = subresourceRange
        };
        VkImageView imageView;
        vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &imageView);
        m_swapchainImageViews.emplace_back(imageView);
    }

    const std::vector<char> code = readFile("shaders/slang.spv");
    VkShaderModule module = createShaderModule(m_device, code);
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

    [[maybe_unused]] VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,fragShaderStageInfo};

    vkDestroyShaderModule(m_device, module , nullptr);
    return 0;
}

void VulkanCore::cleanup() {

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffer);
    std::cout << "Destroyed Command Buffer\n";

    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    std::cout << "Destroyed Command Pool\n";

    for (VkImageView &imageView : m_imageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    std::cout << "Destroyed Image Views\n";
    for (VkImageView &imageView : m_swapchainImageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    std::cout << "Destroyed Swapchain Image Views\n";

    for (VkImage &image : m_images) {
        vkDestroyImage(m_device, image, nullptr);
    }
    m_images.clear();
    std::cout << "Destroyed Images\n";

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    std::cout << "Destroyed Swapchain\n";

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    std::cout << "Destroyed Surface\n";

    delete window;
    std::cout << "Destroyed Window\n";

    // Destroy buffer view
    vkDestroyBufferView(m_device, m_bufferView, nullptr);
    std::cout << "Destoyed Buffer View\n";

    vkDestroyBuffer(m_device, m_buffer, nullptr);
    std::cout << "Destroyed Buffer\n";

    for (VkDeviceMemory &memory : m_memory) {
        vkFreeMemory(m_device, memory, nullptr);
    }
    std::cout << "Deallocated memory\n";

    vkDeviceWaitIdle(m_device);
    vkDestroyDevice(m_device, nullptr);
    std::cout << "Destroyed Device\n";

    vkDestroyInstance(m_instance, nullptr);
    std::cout << "Destroyed Instance\n";
}
