#include "Model.h"
#include "Renderer.h"
#include "Timer.h"

void handleInput(FS::Window &window) {
    FS::Input &input = window.getInput();

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
}
int main() {
    // OBJModel model = {
    //     .vertices = {
    //         { { -0.5f, 0.f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
    //         { {  0.5f, 0.f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    //         { {  0.5f, 0.f,-0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
    //         { { -0.5f, 0.f,-0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },

    //         { { -0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
    //         { {  0.5f, -0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    //         { {  0.5f, -0.5f,-0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
    //         { { -0.5f, -0.5f,-0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }, 
    //     },
    //     .indices = { 
    //         0, 1, 2, 2, 3, 0,
    //         4, 5, 6, 6, 7, 4
    //     }
    // };
    OBJModel model = loadOBJ("models/viking_room.obj",true);

    Timer timer;
    FS::Window window("Vulkan Renderer", 720, 720);

    DeviceContext deviceContext = {};
    TIME_FUNC("initDeviceContext",timer,initDeviceContext(deviceContext, window));

    SwapchainContext swapchainContext = {};
    TIME_FUNC("initSwapchainContext",timer,initSwapchainContext(deviceContext, swapchainContext, window));

    Mesh mesh = {};
    TIME_FUNC("initMesh",timer,initMesh(deviceContext, mesh, model));
    
    Renderer renderer = {};
    TIME_FUNC("initRenderer",timer,initRenderer(deviceContext, swapchainContext, renderer, mesh));

    window.focus();
    while (window.isOpen()) {
        startTimer(timer);
        drawFrame(deviceContext, swapchainContext, window, renderer, mesh);
        handleInput(window);
        window.processMessages();
        endTimer(timer);
        std::cout << "\rFPS: " << microsecToFPS(timer.diff) << std::flush;
    }

    vkDeviceWaitIdle(deviceContext.device);
    cleanupMesh(deviceContext, mesh);
    cleanupRenderer(deviceContext, renderer);
    cleanupSwapchainContext(deviceContext, swapchainContext);
    cleanupDeviceContext(deviceContext);
    window.close();
}