#include "Application.h"
#include <iostream>
#define SELECTED_DEVICE 0
static VkResult createInstance(VkInstance& instance, const char* instanceExtensions[], const char* instanceLayers[]) {
    //Application Info
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = "MyApp";
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = "FSEngine";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    //Instance Create Info
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = 1;
    instanceCreateInfo.ppEnabledLayerNames = instanceLayers;
    instanceCreateInfo.enabledExtensionCount = 2;
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;

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
VkResult createDevice(VkPhysicalDevice& physicalDevice, VkDevice& device, const char* deviceLayers[],const char* deviceExtensions[]) {

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
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.queueFamilyIndex = familyIndex;
    queueCreateInfo.pQueuePriorities = &priority;

    //Device Create Info
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledLayerCount = 1;
    deviceCreateInfo.ppEnabledLayerNames = deviceLayers;
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
    deviceCreateInfo.pEnabledFeatures = &requiredFeatures;

    return vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
}

void printInstanceLayersAndExt() {
    uint32_t propertyCount = 0;
    std::vector<VkLayerProperties> layerProperties = {};
    vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);

    layerProperties.resize(propertyCount);
    vkEnumerateInstanceLayerProperties(&propertyCount, layerProperties.data());
    std::cout << "Instance Layers:" << propertyCount << "\n";
    // for (int i = 0;i < propertyCount;i++) {
    //     std::cout << layerProperties[i].layerName << "\n";
    // }

    propertyCount = 0;
    std::vector<VkExtensionProperties> extensionProperties = {};
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);

    extensionProperties.resize(propertyCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, extensionProperties.data());
    std::cout << "Instance Extensions:" << propertyCount << "\n";
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
    std::cout << "Device Layers:" << propertyCount << "\n";
    // for (int i = 0;i < propertyCount;i++) {
    //     std::cout << layerProperties[i].layerName << "\n";
    // }

    propertyCount = 0;
    std::vector<VkExtensionProperties> extensionProperties = {};
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr);

    extensionProperties.resize(propertyCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, extensionProperties.data());

    std::cout << "Device Extensions:" << propertyCount << "\n";
    // for (int i = 0;i < propertyCount;i++) {
    //     std::cout << extensionProperties[i].extensionName << "\n";
    // }
}

VkResult createBuffer(VkDevice& device, VkBuffer& buffer) {
    static const VkBufferCreateInfo bufferCreateInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        1024 * 1024,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0 , nullptr
    };

    return vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
}
VkResult createSurface(const VkInstance& instance, HWND& windowHandle, VkSurfaceKHR& surface) {
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.hwnd = windowHandle;
    surfaceCreateInfo.hinstance = GetModuleHandleA(nullptr);
    return vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#elif __linux__
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    return vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#endif
}
VkResult createSwapchain(const VkPhysicalDevice& physicalDevice, const VkDevice& device, const VkSurfaceKHR& surface, VkSwapchainKHR& swapchain) {
    uint32_t formatCount;
    std::vector<VkSurfaceFormatKHR> formats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
    VkSurfaceFormatKHR chosenFormat = formats[0];

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = (caps.minImageCount <= 2) ? 2 : caps.minImageCount;
    swapchainCreateInfo.imageFormat = chosenFormat.format;
    swapchainCreateInfo.imageColorSpace = chosenFormat.colorSpace;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageExtent = caps.currentExtent;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    swapchainCreateInfo.preTransform = caps.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    return vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
}
void Application::cleanup() {
    vkDestroySwapchainKHR(m_device,m_swapchain,nullptr);
    vkDestroySurfaceKHR(m_instance,m_surface,nullptr);
    vkDestroyBuffer(m_device,m_buffer,nullptr);
    vkDeviceWaitIdle(m_device);
    vkDestroyDevice(m_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}
LRESULT windowProcedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProcA(window, msg, wParam, lParam);
}
int Application::init() {
    if (createInstance(m_instance, instanceExtensions, instanceLayers) != VK_SUCCESS) {
        std::cerr << "Failed to create Instance\n";
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


    if (createDevice(m_physicalDevice, m_device, deviceLayers,deviceExtensions) != VK_SUCCESS) {
        std::cerr << "Failed to create Logical Device\n";
        return 1;
    }
    else {
        std::cout << "Successfully created Logical Device\n";
    }


    printInstanceLayersAndExt();
    printDeviceLayersAndExt(m_physicalDevice);

    if (createBuffer(m_device, m_buffer) != VK_SUCCESS) {
        std::cerr << "Failed to create buffer\n";
        return 1;
    }
    else {
        std::cout << "Successfully created buffer\n";
    }

#ifdef _WIN32
    WNDCLASSA mWindowClass = {};
    mWindowClass.lpfnWndProc = windowProcedure;
    mWindowClass.lpszClassName = "ClassName";
    mWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    mWindowClass.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClassA(&mWindowClass);

    HWND window = CreateWindowA(mWindowClass.lpszClassName, "My lil Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 720, 720, NULL, NULL, NULL, NULL);
#endif

    VkResult result;
    if (createSurface(m_instance, window, m_surface) != VK_SUCCESS) {
        std::cerr << "Failed to create Surface\n";
        return 1;
    }
    else {
        std::cout << "Successfully created surface\n";
    }

    if (createSwapchain(m_physicalDevice, m_device, m_surface, m_swapchain) != VK_SUCCESS) {
        std::cerr << "Failed to create Swapchain\n";
        return 1;
    }
    else {
        std::cout << "Successfully created Swapchain\n";
    }
    return VK_SUCCESS;
}
void Application::run() {
    init();
    cleanup();
}