#include "ImGuiLayer.h"
#include <vector>

void ImGuiLayer::init(DeviceContext &deviceContext, SwapchainContext &swapchainContext) {
    mDeviceContext                             = deviceContext;
    std::vector<VkDescriptorPoolSize> poolSize = {
        VkDescriptorPoolSize{
            .type            = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE },
        VkDescriptorPoolSize{
            .type            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE,
        },
    };
    mDescriptorPool = createDescriptorPool(deviceContext, poolSize);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplVulkan_InitInfo imguiInfo{};
    imguiInfo.Instance                                                             = deviceContext.instance;
    imguiInfo.PhysicalDevice                                                       = deviceContext.physicalDevice;
    imguiInfo.Device                                                               = deviceContext.device;
    imguiInfo.QueueFamily                                                          = getQueueFamilyIndex(deviceContext, VK_QUEUE_GRAPHICS_BIT);
    imguiInfo.Queue                                                                = deviceContext.graphicsQueue;
    imguiInfo.DescriptorPool                                                       = mDescriptorPool;
    imguiInfo.MinImageCount                                                        = 2;
    imguiInfo.ImageCount                                                           = swapchainContext.imageCount;
    imguiInfo.UseDynamicRendering                                                  = true;
    imguiInfo.PipelineInfoMain.PipelineRenderingCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    imguiInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount    = 1;
    imguiInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchainContext.surfaceFormat.format;
    imguiInfo.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat   = VK_FORMAT_D32_SFLOAT;

    ImGui_ImplVulkan_Init(&imguiInfo);

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsClassic();
}
void ImGuiLayer::shutdown() {
    ImGui_ImplVulkan_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(mDeviceContext.device, mDescriptorPool, nullptr);
}
void ImGuiLayer::beginFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}
void ImGuiLayer::endFrame() {
    ImGui::EndFrame();
}
void ImGuiLayer::render(VkCommandBuffer commandBuffer) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
void ImGuiLayer::handleInput(FS::Window &window) {
    FS::RenderState &renderState = window.getRenderState();
    FS::Input &input             = window.getInput();
    ImGuiIO &io                  = ImGui::GetIO();
    io.DisplaySize               = ImVec2(
        static_cast<float>(renderState.width),
        static_cast<float>(renderState.height));

    io.DisplayFramebufferScale = ImVec2(
        1.f, 1.f);

    FS::Vector2 cursorPos = window.getCursorPos(true);
    bool leftMouseDown    = isDown(FS::Buttons::MOUSE_BUTTON_LEFT);
    bool rightMouseDown   = isDown(FS::Buttons::MOUSE_BUTTON_RIGHT);
    io.AddMousePosEvent(cursorPos.x, cursorPos.y);
    io.AddMouseButtonEvent(0, leftMouseDown);
    io.AddMouseButtonEvent(1, rightMouseDown);
}