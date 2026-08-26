#include "Logging.h"
#include "Instance.h"
#include <cstdint>
#include <iostream>
VulkanInstance::VulkanInstance() {
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = this->appName.c_str(),
        .applicationVersion = this->appVersion,
        .pEngineName = this->engineName.c_str(),
        .engineVersion = this->engineVersion,
        .apiVersion = this->apiVersion
    };
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
        .ppEnabledLayerNames = enabledLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
        .ppEnabledExtensionNames = enabledExtensions.data()
    };
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    LOG_CREATION(result,"Instance");
}
VulkanInstance::~VulkanInstance(){
    vkDestroyInstance(m_instance, nullptr);
    std::cout << "Destroyed Instance\n";
}

VkPhysicalDevice VulkanInstance::getPhysicalDevice(uint32_t index){
    uint32_t physicalDeviceCount=0;
    vkEnumeratePhysicalDevices(this->m_instance, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(this->m_instance, &physicalDeviceCount, physicalDevices.data());

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevices[index], &physicalDeviceProperties);
    std::cout << "Selected Device: " << physicalDeviceProperties.deviceName << "\n";
    return physicalDevices[index];
}