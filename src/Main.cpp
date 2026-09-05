#include "Renderer.h"
#include "Model.h"
void handleInput(FS::Window &window) {
    FS::Input &input = window.getInput();

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
}
int main() {

    OBJModel model = loadOBJ("models/king.obj");

    FS::Window window("Vulkan Renderer", 720, 720);

    DeviceContext deviceContext = {};
    initDeviceContext(deviceContext, window);

    SwapchainContext swapchainContext = {};
    initSwapchainContext(deviceContext, swapchainContext, window);

    Renderer renderer = {};
    initRenderer(deviceContext, swapchainContext, renderer);

    Mesh mesh = {};
    initMesh(deviceContext, mesh, model.vertices.data(), static_cast<uint32_t>(model.vertices.size()), model.indices.data(), static_cast<uint32_t>(model.indices.size()));

    while (window.isOpen()) {
        drawFrame(deviceContext, swapchainContext, window, renderer, mesh);
        handleInput(window);
        window.processMessages();
    }

    vkDeviceWaitIdle(deviceContext.device);
    cleanupMesh(deviceContext, mesh);
    cleanupRenderer(deviceContext, renderer);
    cleanupSwapchainContext(deviceContext, swapchainContext);
    cleanupDeviceContext(deviceContext);
    window.close();
}