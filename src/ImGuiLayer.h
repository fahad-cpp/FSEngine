#ifndef IMGUILAYER_H
#define IMGUILAYER_H
#include "vulkan/SwapchainContext.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>

class ImGuiLayer {
    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    DeviceContext mDeviceContext{};

  public:
    void init(DeviceContext &deviceContext, SwapchainContext &swapchainContext);
    void shutdown();
    void beginFrame();
    void endFrame();
    void render(VkCommandBuffer);
    void handleInput(FS::Window &window);
};
#endif