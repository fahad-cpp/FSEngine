#include "vulkan/Renderer.h"
#include "Timer.h"

void handleInput(FS::Window &window) {
    FS::Input &input = window.getInput();

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
}
int main() {
    OBJModel model = loadOBJ("models/Zenith.obj",true);

    Timer timer;
    FS::Window window("FSEngine", 720, 720);
    LOG_ERROR("Test error");

    DeviceContext deviceContext = {};
    TIME_FUNC("initDeviceContext",timer,initDeviceContext(deviceContext, window));

    SwapchainContext swapchainContext = {};
    TIME_FUNC("initSwapchainContext",timer,initSwapchainContext(deviceContext, swapchainContext, window));

    Mesh mesh = {};
    TIME_FUNC("createMesh",timer,mesh = createMesh(deviceContext, model));
    
    VulkanRenderer renderer = {};
    TIME_FUNC("initVulkanRenderer",timer,initVulkanRenderer(deviceContext, swapchainContext, renderer, mesh));
    window.focus();
    while (window.isOpen()) {
        startTimer(timer);
        drawFrame(deviceContext, swapchainContext, window, renderer, mesh);
        handleInput(window);
        window.processMessages();
        endTimer(timer);
        LOG_LIVE("FPS: " << microsecToFPS(timer.diff));
    }
    vkDeviceWaitIdle(deviceContext.device);
    cleanupMesh(deviceContext, mesh);
    cleanupVulkanRenderer(deviceContext, renderer);
    cleanupSwapchainContext(deviceContext, swapchainContext);
    cleanupDeviceContext(deviceContext);
    window.close();
}