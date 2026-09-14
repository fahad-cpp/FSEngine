#include "vulkan/Renderer.h"
#include "Timer.h"
#include <numbers>
void handleInput(FS::Window &window,Camera& camera) {
    FS::Input &input = window.getInput();

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }

    float moveSpeed = 0.01f;
    if(isDown(FS::Buttons::BUTTON_W)){
        Vector3 rotated = rotate(Vector3{0.f,0.f,moveSpeed},camera.rotation.y,Vector3{0.f,1.f,0.f});
        camera.position.x += rotated.x;
        camera.position.y += rotated.y;
        camera.position.z += rotated.z;
    }
    if(isDown(FS::Buttons::BUTTON_S)){
        Vector3 rotated = rotate(Vector3{0.f,0.f,-moveSpeed},camera.rotation.y,Vector3{0.f,1.f,0.f});
        camera.position.x += rotated.x;
        camera.position.y += rotated.y;
        camera.position.z += rotated.z;
    }
    if(isDown(FS::Buttons::BUTTON_A)){
        Vector3 rotated = rotate(Vector3{-moveSpeed,0.f,0.f},camera.rotation.y,Vector3{0.f,1.f,0.f});
        camera.position.x += rotated.x;
        camera.position.y += rotated.y;
        camera.position.z += rotated.z;
    }
    if(isDown(FS::Buttons::BUTTON_D)){
        Vector3 rotated = rotate(Vector3{moveSpeed,0.f,0.f},camera.rotation.y,Vector3{0.f,1.f,0.f});
        camera.position.x += rotated.x;
        camera.position.y += rotated.y;
        camera.position.z += rotated.z;
    }

    if(isDown(FS::Buttons::BUTTON_SPACE)){
        camera.position.y += moveSpeed;
    }
    if(isDown(FS::Buttons::BUTTON_CTRL)){
        camera.position.y -= moveSpeed;
    }

    float pi = static_cast<float>(std::numbers::pi);
    float rotateSpeed = 0.05f;
    if(isDown(FS::Buttons::BUTTON_UP)){
        camera.rotation.x -= rotateSpeed / pi;
    }
    if(isDown(FS::Buttons::BUTTON_DOWN)){
        camera.rotation.x += rotateSpeed / pi;
    }
    if(isDown(FS::Buttons::BUTTON_LEFT)){
        camera.rotation.y -= rotateSpeed / pi;
    }
    if(isDown(FS::Buttons::BUTTON_RIGHT)){
        camera.rotation.y += rotateSpeed / pi;
    }

}
int main() {
    OBJModel model = loadOBJ("models/viking_room.obj",true);

    Timer timer;
    FS::Window window("FSEngine", 720, 720);

    DeviceContext deviceContext = {};
    TIME_FUNC("initDeviceContext",timer,initDeviceContext(deviceContext, window));

    SwapchainContext swapchainContext = {};
    TIME_FUNC("initSwapchainContext",timer,initSwapchainContext(deviceContext, swapchainContext, window));

    Mesh mesh = {};
    TIME_FUNC("createMesh",timer,mesh = createMesh(deviceContext, model));
    
    VulkanRenderer renderer = {};
    TIME_FUNC("initVulkanRenderer",timer,initVulkanRenderer(deviceContext, swapchainContext, renderer, mesh));
    window.focus();

    Camera camera = {
        .position = {0.f,0.f,0.f},
        .rotation = {0.f,0.f,0.f}
    };

    while (window.isOpen()) {
        startTimer(timer);
        drawFrame(deviceContext, swapchainContext, window, renderer, mesh,camera);
        handleInput(window,camera);
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