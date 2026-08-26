#include "Command.h"
#include "Logging.h"

VulkanCommandPool::VulkanCommandPool(VulkanDevice& device,uint32_t queueFamilyIndex,VkCommandPoolCreateFlags flags){
    m_device = device.get();
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
        .queueFamilyIndex = queueFamilyIndex
    };
    VkResult result = vkCreateCommandPool(device.get(), &commandPoolCreateInfo,nullptr,&m_commandPool);
    LOG_CREATION(result,"Command Pool");
    
}
VulkanCommandPool::~VulkanCommandPool(){
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    std::cout << "Destroyed Command Pool\n";
}
std::vector<VkCommandBuffer> VulkanCommandPool::createCommandBuffers(uint32_t count){
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count
    };
    std::vector<VkCommandBuffer> commandBuffers(count);
    vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, commandBuffers.data());
    return commandBuffers;
}