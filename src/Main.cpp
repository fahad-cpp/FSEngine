#include "Model.h"
#include "Renderer.h"
#include "Timer.h"
#include <iostream>

void handleInput(FS::Window &window) {
    FS::Input &input = window.getInput();

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
}
int main() {
    // OBJModel model = loadOBJ("models/cube.obj");
    OBJModel model = {
        .vertices = {
            { { -0.5f, 0.f, 0.5f }, { 1.0f, 0.0f, 0.0f } },
            { { 0.5f, 0.f,  0.5f}, { 0.0f, 1.0f, 0.0f } },
            { { 0.5f, 0.f ,-0.5f}, { 0.0f, 0.0f, 1.0f } },
            { { -0.5f, 0.f ,-0.5f}, { 1.0f, 1.0f, 1.0f } } },
        .indices = { 0, 1, 2, 2, 3, 0 }
    };

    Timer timer;
    startTimer(timer);
    FS::Window window("Vulkan Renderer", 720, 720);

    DeviceContext deviceContext = {};
    initDeviceContext(deviceContext, window);

    SwapchainContext swapchainContext = {};
    initSwapchainContext(deviceContext, swapchainContext, window);

    Renderer renderer = {};
    initRenderer(deviceContext, swapchainContext, renderer);

    Mesh mesh = {};
    initMesh(deviceContext, mesh, model.vertices.data(), static_cast<uint32_t>(model.vertices.size()), model.indices.data(), static_cast<uint32_t>(model.indices.size()));
    endTimer(timer);

    std::cout << "Setup took: " << timer.diff / 1000 << " ms\n";
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