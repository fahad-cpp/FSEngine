#include "Renderer.h"
#include "Utilities.h"
struct UniformBufferObject {
    Matrix4 model;
    Matrix4 view;
    Matrix4 projection;
};

void handleInput(FS::Window &window) {
    FS::Input &input = window.getInput();

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
}
int main() {

    const Vertex vertices[] = {
        { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
        { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
        { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
    };
    const uint32_t indices[] = {
        0, 1, 2, 2, 3, 0
    };

    FS::Window window("Vulkan Renderer", 720, 720);

    DeviceContext deviceContext = {};
    initDeviceContext(deviceContext, window);

    SwapchainContext swapchainContext = {};
    initSwapchainContext(deviceContext, swapchainContext, window);

    Renderer renderer = {};
    initRenderer(deviceContext, swapchainContext, renderer);

    uint32_t vertexCount = sizeof(vertices) / sizeof(vertices[0]);
    uint32_t indexCount = sizeof(indices) / sizeof(indices[0]);
    Mesh mesh = {};
    initMesh(deviceContext, mesh, vertices, vertexCount, indices, indexCount);

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