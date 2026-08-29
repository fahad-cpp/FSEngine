#include "Buffers.h"
#include "Command.h"
#include "Device.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Renderer.h"
#include "Surface.h"
#include "Swapchain.h"
#include <FSWindow.h>

const std::vector<Vertex> vertices = {
    { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
};
const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0
};
void handleInput(FS::Window &window) {
    FS::Input &input = window.getInput();
    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
}
int main() {
    VulkanInstance instance;

    VulkanPhysicalDevice physicalDevice(instance.getPhysicalDevice());

    VulkanDevice device(physicalDevice, { VK_QUEUE_GRAPHICS_BIT });

    FS::Window window("FSWindow::Window", 720, 720);
    VulkanSurface surface(instance, physicalDevice, window);

    VulkanSwapchain swapchain(device, surface, window);

    const uint32_t graphicsQueueFamilyIndex = physicalDevice.getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

    VulkanCommandPool commandPool(device, graphicsQueueFamilyIndex, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBuffer commandBuffer = commandPool.createCommandBuffers(1)[0];

    VertexBuffer vertexBuffer(device, commandBuffer, physicalDevice, vertices);
    IndexBuffer indexBuffer(device, commandBuffer, physicalDevice, indices);

    VulkanPipeline graphicsPipeline(device, swapchain, "shaders/slang.spv", vertexBuffer, indexBuffer);

    const uint32_t framesInFlight = 2;
    BackBufferState backBufferState = {
        .commandBuffers = commandPool.createCommandBuffers(framesInFlight),
        .acquireSemaphores = device.createSemaphores(framesInFlight),
        .renderSemaphores = device.createSemaphores(static_cast<uint32_t>(swapchain.getImages().size())),
        .frameFences = device.createFences(framesInFlight, VK_FENCE_CREATE_SIGNALED_BIT)
    };

    Renderer renderer(device, physicalDevice, swapchain, backBufferState, graphicsPipeline);
    renderer.setVertices(vertexBuffer);
    renderer.setIndices(indexBuffer);

    while (window.isOpen()) {
        renderer.drawFrame();
        handleInput(window);
        window.processMessages();
    }
}