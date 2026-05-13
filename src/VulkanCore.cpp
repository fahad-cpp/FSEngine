#include "VulkanCore.h"
#include <iostream>
#define SELECTED_DEVICE 0
static VkResult createInstance(VkInstance& instance, const std::vector<const char*> instanceExtensions, const std::vector<const char*> instanceLayers) {
    //Application Info
    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "MyApp",
        .applicationVersion = 1,
        .pEngineName = "FSEngine",
        .engineVersion = 1,
        .apiVersion = VK_MAKE_VERSION(1, 0, 0)
    };

    //Instance Create Info
    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = (uint32_t)instanceLayers.size(),
        .ppEnabledLayerNames = instanceLayers.data(),
        .enabledExtensionCount = (uint32_t)instanceExtensions.size(),
        .ppEnabledExtensionNames = instanceExtensions.data()
    };

    //Create the Instance
    return vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
}
VkResult getPhysicalDevice(VkInstance& instance, VkPhysicalDevice& physicalDevice) {
    //Enumerate Device
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

    //Select a device
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
void getPhysicalDeviceProperties(VkPhysicalDevice& physicalDevice) {
    //PhysicalDeviceProperties
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    std::cout << "Selected Device: ";
    std::cout << properties.deviceName << "\n";

    //PhysicalDeviceMemoryProperties
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

    //PhysicalDeviceQueueFamilyProperties
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
uint32_t getQueueFamilyIndex(const VkPhysicalDevice& physicalDevice) {
    uint32_t index = 0;
    uint32_t propertyCount = 0;
    std::vector<VkQueueFamilyProperties> properties;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propertyCount, nullptr);
    properties.resize(propertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propertyCount, properties.data());

    for (uint32_t i = 0;i < propertyCount;i++) {
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            index = i;
            break;
        }
    }

    return index;
}
VkResult createDevice(VkPhysicalDevice& physicalDevice, VkDevice& device, const std::vector<const char*> deviceLayers, const std::vector<const char*> deviceExtensions) {

    uint32_t familyIndex = getQueueFamilyIndex(physicalDevice);

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    //Specify required Features 
    //An Example where tesselation shader and geometry shaders are must have
    //and multiDrawIndirect is supported if the device supports it
    VkPhysicalDeviceFeatures requiredFeatures = {};
    requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
    requiredFeatures.tessellationShader = VK_TRUE;
    requiredFeatures.geometryShader = VK_TRUE;

    //Queue Create Info
    float priority = 1.f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = familyIndex,
        .queueCount = 1,
        .pQueuePriorities = &priority
    };

    //Device Create Info
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledLayerCount = (uint32_t)deviceLayers.size(),
        .ppEnabledLayerNames = deviceLayers.data(),
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
    //std::cout << "Instance Layers:" << propertyCount << "\n";
    // for (int i = 0;i < propertyCount;i++) {
    //     std::cout << layerProperties[i].layerName << "\n";
    // }

    propertyCount = 0;
    std::vector<VkExtensionProperties> extensionProperties = {};
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);

    extensionProperties.resize(propertyCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, extensionProperties.data());
    //std::cout << "Instance Extensions:" << propertyCount << "\n";
    // for (int i = 0;i < propertyCount;i++) {
    //     std::cout << extensionProperties[i].extensionName << "\n";
    // }
}
void printDeviceLayersAndExt(VkPhysicalDevice& physicalDevice) {
    uint32_t propertyCount = 0;
    std::vector<VkLayerProperties> layerProperties = {};
    vkEnumerateDeviceLayerProperties(physicalDevice, &propertyCount, nullptr);

    layerProperties.resize(propertyCount);
    vkEnumerateDeviceLayerProperties(physicalDevice, &propertyCount, layerProperties.data());
    // std::cout << "Device Layers:" << propertyCount << "\n";
    // for (int i = 0;i < propertyCount;i++) {
    //     std::cout << layerProperties[i].layerName << "\n";
    // }

    propertyCount = 0;
    std::vector<VkExtensionProperties> extensionProperties = {};
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr);

    extensionProperties.resize(propertyCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, extensionProperties.data());

    // std::cout << "Device Extensions:" << propertyCount << "\n";
    // for (int i = 0;i < propertyCount;i++) {
    //     std::cout << extensionProperties[i].extensionName << "\n";
    // }
}

#ifdef _WIN32
VkResult createSurface(const VkInstance& instance, HWND& windowHandle, VkSurfaceKHR& surface) {
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .hinstance = GetModuleHandleA(nullptr),
        .hwnd = windowHandle
    };
    return vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
}
#elif __linux__
VkResult createSurface(const VkInstance& instance, Window& windowHandle, VkSurfaceKHR& surface) {
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .dpy = XOpenDisplay(0),
        .window = windowHandle
    };
    return vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
}
#endif
VkResult createSwapchain(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkSurfaceKHR& surface, VkSwapchainKHR& swapchain) {
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

//Resources creation
VkResult createBuffer(VkDevice& device, VkBuffer& buffer) {
    static const VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = 1024 * 1024,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0 ,
        .pQueueFamilyIndices = nullptr
    };

    return vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
}

VkResult createImage(VkPhysicalDevice& physicalDevice, VkDevice& device, VkImage& image) {
    VkFormat selectedFormat = VK_FORMAT_R8G8B8A8_UNORM;
    
    //Optimal Image
    // static const VkImageCreateInfo imageCreateInfo = {
    //     .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    //     .pNext = nullptr,
    //     .flags = 0,
    //     .imageType = VK_IMAGE_TYPE_2D,
    //     .format = selectedFormat,
    //     .extent = VkExtent3D{1920,1080,1},
    //     .mipLevels = 10,
    //     .arrayLayers = 1,
    //     .samples = VK_SAMPLE_COUNT_1_BIT,
    //     .tiling = VK_IMAGE_TILING_OPTIMAL,
    //     .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
    //     .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    //     .queueFamilyIndexCount = 0,
    //     .pQueueFamilyIndices = nullptr,
    //     .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    // };

    //Linear Image
    static const VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = selectedFormat,
        .extent = VkExtent3D{1920,1080,1},
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

    std::cout << "offset:" << subResLayout.offset << "\n";
    std::cout << "size:" << subResLayout.size << "\n";
    std::cout << "rowPitch:" << subResLayout.rowPitch << "\n";
    std::cout << "arrayPitch:" << subResLayout.arrayPitch << "\n";
    std::cout << "depthPitch:" << subResLayout.depthPitch << "\n";

    return res;
}

int VulkanCore::init() {
    VkResult result = createInstance(m_instance, instanceExtensions, instanceLayers);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Instance:" << result << "\n";
        std::cerr << result;
        return 1;
    }
    else {
        std::cout << "Successfully created Instance\n";
    }

    if (getPhysicalDevice(m_instance, m_physicalDevice) != VK_SUCCESS) {
        std::cerr << "Failed to query Physical Device\n";
        return 1;
    }
    else {
        std::cout << "Successfully Received Physical Device\n";
    }

    getPhysicalDeviceProperties(m_physicalDevice);


    if ((result = createDevice(m_physicalDevice, m_device, deviceLayers, deviceExtensions)) != VK_SUCCESS) {
        std::cerr << "Failed to create Logical Device:" << result << "\n";
        return 1;
    }
    else {
        std::cout << "Successfully created Logical Device\n";
    }


    printInstanceLayersAndExt();
    printDeviceLayersAndExt(m_physicalDevice);

    if ((result = createBuffer(m_device, m_buffer)) != VK_SUCCESS) {
        std::cerr << "Failed to create buffer:" << result << "\n";
        return 1;
    }
    else {
        std::cout << "Successfully created buffer\n";
    }

#ifdef _WIN32
    m_window = createWin32Window();
    if (m_window) {
        std::cout << "Successfully created Window\n";
    }
    else {
        std::cerr << "Failed to create Window\n";
        return 1;
    }
#elif __linux__
    m_window = createXlibWindow();
    if (m_window) {
        std::cout << "Successfully created Window\n";
    }
    else {
        std::cerr << "Failed to create Window\n";
        return 1;
    }
#endif

    if ((result = createSurface(m_instance, m_window, m_surface)) != VK_SUCCESS) {
        std::cerr << "Failed to create Surface:" << result << "\n";
        return 1;
    }
    else {
        std::cout << "Successfully created surface\n";
    }

    if ((result = createSwapchain(m_physicalDevice, m_device, m_surface, m_swapchain)) != VK_SUCCESS) {
        std::cerr << "Failed to create Swapchain:" << result << "\n";
        return 1;
    }
    else {
        std::cout << "Successfully created Swapchain\n";
    }

    result = createImage(m_physicalDevice, m_device, m_image);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Image:" << result << "\n";
        return 1;
    }
    else {
        std::cout << "Successfully created Image\n";
    }
    
    //Query Compressed formats support
    VkPhysicalDeviceFeatures feat;
    vkGetPhysicalDeviceFeatures(m_physicalDevice,&feat);
    std::cout << ((feat.textureCompressionBC)? "BC texture compression supported\n" : "BC texture compression not supported\n");
    std::cout << ((feat.textureCompressionETC2)? "ETC2 texture compression supported\n" : "ETC2 texture compression not supported\n");
    std::cout << ((feat.textureCompressionASTC_LDR)? "ASTC texture compression supported\n" : "ASTC texture compression not supported\n");
    
    
    return VK_SUCCESS;
}

void VulkanCore::cleanup() {
    vkDestroyImage(m_device, m_image, nullptr);
    std::cout << "Destroyed Image\n";
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    std::cout << "Destroyed Swapchain\n";
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    std::cout << "Destroyed Surface\n";
#ifdef _WIN32
    DestroyWindow(m_window);
#elif __linux__
    deleteXlibWindow(m_window);
#endif
    std::cout << "Destroyed Window\n";
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    std::cout << "Destroyed Buffer\n";
    vkDeviceWaitIdle(m_device);
    vkDestroyDevice(m_device, nullptr);
    std::cout << "Destroyed Device\n";
    vkDestroyInstance(m_instance, nullptr);
    std::cout << "Destroyed Instance\n";
}