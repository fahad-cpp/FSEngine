# Chapter 1  : Overview of vulkan


## Listing 1.1: Creating a Vulkan Instance
```C++
VkResult createInstance(VkInstance& instance){
    //Application Info
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = "MyApp";
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = "FSEngine";
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1,0,0);

    //Instance Create Info
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;
    instanceCreateInfo.enabledExtensionCount = 0;
    instanceCreateInfo.ppEnabledExtensionNames = nullptr;

    //Create the Instance
    VkResult result = vkCreateInstance(&instanceCreateInfo,nullptr,&instance);

    if(result == VK_SUCCESS){
        std::cout << "Successfully Created Vulkan Instace\n";
    }else{
        std::cerr << "Failed to Create Vulkan Instance\n";
    }

    return result;
}
VkResult getPhysicalDevices(VkInstance& instance,std::vector<VkPhysicalDevice>& physicalDevices){
    VkResult result = VK_SUCCESS;
    uint32_t physicalDeviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance,&physicalDeviceCount,nullptr);
    if(result != VK_SUCCESS){
        std::cerr << "Failed to query Physical Devices\n";
        return result;
    }

    physicalDevices.resize(physicalDeviceCount);
    result = vkEnumeratePhysicalDevices(instance,&physicalDeviceCount,physicalDevices.data());
    if(result != VK_SUCCESS){
        std::cerr << "Failed to receive Physical Devices\n";
        return result;
    }
    std::cout << "Successfully received Physical Devices\n";

    return VK_SUCCESS;
}

VkResult Application::init(){
    VkResult result = createInstance(m_instance);
    if(result != VK_SUCCESS){
        return result;
    }

    result = getPhysicalDevices(m_instance,m_physicalDevices);
    if(result != VK_SUCCESS){
        return result;
    }

    return VK_SUCCESS;
}
```

## Listing 1.2 Querying Physical Device Properties


```C++
void getPhysicalDeviceProperties(std::vector<VkPhysicalDevice>& physicalDevices){
    std::cout << "Physical Devices:\n";
    for(int i=0;i<physicalDevices.size();i++){
        //PhysicalDeviceProperties
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[i],&properties);
        std::cout << i+1 << '.' << properties.deviceName << "\n";

        //PhysicalDeviceMemoryProperties
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevices[i],&memoryProperties);
        std::cout << "\tMemory Types:\n";
        for(int j=0;j<memoryProperties.memoryTypeCount;j++){
            std::cout <<"\t\t" << "Memory Type "<<j << ":" << getMemoryPropertyFlagString(memoryProperties.memoryTypes[j].propertyFlags) << "\n";
        }

        std::cout << "\tHeaps:\n";
        for(int j=0;j<memoryProperties.memoryHeapCount;j++){
            std::cout <<"\t\t" << "Heap "<<j<<"\n";
            std::cout <<"\t\t Heap Size:" << memoryProperties.memoryHeaps[j].size << " Bytes\n";
            std::cout <<"\t\t Heap Flags:" << getMemoryHeapFlagString(memoryProperties.memoryHeaps[j].flags) << "\n";
        }

        //PhysicalDeviceQueueFamilyProperties
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        uint32_t qfpropertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i],&qfpropertyCount,nullptr);
        queueFamilyProperties.resize(qfpropertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i],&qfpropertyCount,queueFamilyProperties.data());

        std::cout << "\tQueue Family Properties:\n";

        for(int j=0;j<qfpropertyCount;j++){
            std::cout << "\t\tQueue Count:" << queueFamilyProperties[j].queueCount << "\n";
            std::cout << "\t\tTimeStamp Valid Bits:" << queueFamilyProperties[j].timestampValidBits << "\n";
            std::cout << "\t\tMin Image Transfer Granularity:\n"
            << "\t\t\tDepth:" << queueFamilyProperties[j].minImageTransferGranularity.depth << "\n"
            << "\t\t\tWidth:" << queueFamilyProperties[j].minImageTransferGranularity.width << "\n"
            << "\t\t\tHeight:" << queueFamilyProperties[j].minImageTransferGranularity.height << "\n";
            std::cout << "\t\tQueue Flags:" << getQueueFlagString(queueFamilyProperties[j].queueFlags) << "\n";
        }
    }
}
```

## Listing 1.3 Creating a Logical Device

```C++
uint32_t getQueueFamilyIndex(VkPhysicalDevice& physicalDevice){
    uint32_t index = 0;
    uint32_t propertyCount=0;
    std::vector<VkQueueFamilyProperties> properties;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&propertyCount,nullptr);
    properties.resize(propertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&propertyCount,properties.data());

    for(uint32_t i=0;i<propertyCount;i++){
        if(properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            index = i;
            break;
        }
    }

    return index;
}
VkResult createLogicalDevice(VkPhysicalDevice& physicalDevice,VkDevice& device){

    uint32_t familyIndex = getQueueFamilyIndex(physicalDevice);

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice,&supportedFeatures);
    
    //Specify required Features 
    //An Example where tesselation shader and geometry shaders are must have
    //and multiDrawIndirect is supported if the device supports it
    VkPhysicalDeviceFeatures requiredFeatures = {};
    requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
    requiredFeatures.tessellationShader = VK_TRUE;
    requiredFeatures.geometryShader = VK_TRUE;

    //Queue Create Info
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.queueFamilyIndex = familyIndex;
    queueCreateInfo.pQueuePriorities = nullptr;

    //Device Create Info
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = 0;
    deviceCreateInfo.ppEnabledExtensionNames = nullptr;
    deviceCreateInfo.pEnabledFeatures = &requiredFeatures;

    VkResult result = vkCreateDevice(physicalDevice,&deviceCreateInfo,nullptr,&device);

    if(result != VK_SUCCESS){
        std::cerr << "Failed to create Logical Device\n";
    }else{
        std::cout << "Successfully created Logical Device\n";
    }

    return result;
}
```

## Listing 1.4 Querying Instance Layers

```C++
void printInstanceLayers() {
    uint32_t propertyCount = 0;
    std::vector<VkLayerProperties> layerProperties = {};
    vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);

    layerProperties.resize(propertyCount);
    vkEnumerateInstanceLayerProperties(&propertyCount, layerProperties.data());
    std::cout << "Instance Layers:\n";
    for (int i = 0;i < propertyCount;i++) {
        std::cout << layerProperties[i].layerName << "\n";
    }
}
```

## Listing 1.5 Querying Instance Extensions

```C++
    uint32_t propertyCount = 0;
    std::vector<VkExtensionProperties> extensionProperties = {};
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);

    extensionProperties.resize(propertyCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, extensionProperties.data());
    std::cout << "Instance Extensions:\n";
    for (int i = 0;i < propertyCount;i++) {
        std::cout << extensionProperties[i].extensionName << "\n";
    }
```

# Chapter 2 : Memory and Resources

## Listing 2.1 Declaration of a memory allocator

```C++
#include <vulkan/vulkan.h>
class Allocator {
public:
    inline operator VkAllocationCallbacks() const {
        VkAllocationCallbacks vulkanAllocator;

        vulkanAllocator.pUserData = (void*)this;
        vulkanAllocator.pfnAllocation = &allocation;
        vulkanAllocator.pfnReallocation = &reAllocation;
        vulkanAllocator.pfnFree = &free;

        vulkanAllocator.pfnInternalAllocation = nullptr;
        vulkanAllocator.pfnInternalFree = nullptr;

        return vulkanAllocator;
    }

    static void* VKAPI_CALL allocation(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationscope);
    static void* VKAPI_CALL reAllocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
    static void VKAPI_CALL free(void* pUserData, void* pMemory);

    void* allocation(size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
    void* reAllocation(void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
    void free(void* pMemory);
};
```

## Listing 2.2 Implementation of a memory allocator

```C++
#include "Allocator.h"
#include <cstdlib>

void* Allocator::allocation(size_t size, size_t alignment, VkSystemAllocationScope allocationScope){
    return _aligned_malloc(size,alignment);
}

void* Allocator::reAllocation(void*pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope){
    return _aligned_realloc(pOriginal,size,alignment);
}

void Allocator::free(void* pMemory){
    _aligned_free(pMemory);
}

void* Allocator::allocation(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope){
    return static_cast<Allocator*>(pUserData)->allocation(size,alignment,allocationScope);
}

void* Allocator::reAllocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope){
    return static_cast<Allocator*>(pUserData)->reAllocation(pOriginal,size,alignment,allocationScope);
}

void Allocator::free(void* pUserData, void* pMemory){
    static_cast<Allocator*>(pUserData)->free(pMemory);
}
```

## Listing 2.3 Creating a Buffer Object
```cpp
VkBuffer createBuffer(VkDevice& device, VkBuffer& buffer) {
    static const VkBufferCreateInfo bufferCreateInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        1024 * 1024,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0 , nullptr
    };

    vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
}
VkResult Application::init(){
    ...
    ...
    
    VkBuffer buffer = VK_NULL_HANDLE;
    createBuffer(m_device, buffer);

    return VK_SUCCESS;
}
```