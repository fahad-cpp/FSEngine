#include "AssetManager.h"
#include "Timer.h"
#include "vulkan/Renderer.h"
#include <algorithm>
#include <numbers>

/*
    TODO:
    -Restructure code according to Design.md
    -MSAA
    -proper light handling (send light array from CPU to GPU)
    -Multi Draw Indirect
    -Push Constants
    -Editor
    -glTF/glb support and PBR
    -GPU-Driven Rendering
*/
static Vector2 getMouseDiff(FS::Window &window) {
    if (!window.isFocused()) {
        return { 0.f, 0.f };
    }
    static FS::RenderState &renderState = window.getRenderState();
    FS::Vector2 windowPos               = window.getWindowPos();
    FS::Vector2 mousePos                = window.getCursorPos();
    FS::Vector2 centerPos               = { (windowPos.x + static_cast<float>(renderState.width) / 2.f), (windowPos.y + static_cast<float>(renderState.height) / 2.f) };
    FS::Vector2 diff                    = mousePos - centerPos;
    window.setCursorPos(static_cast<uint32_t>(centerPos.x), static_cast<uint32_t>(centerPos.y));
    return { diff.x, diff.y };
}
void cameraMovementSystem(Camera &camera, FS::Window &window,float dt) {
    float moveSpeed   = 0.01f * dt;
    FS::Input &input  = window.getInput();
    Vector2 mouseDiff = getMouseDiff(window);

    camera.rotation.x += mouseDiff.y * 0.001f;
    camera.rotation.x = std::clamp(camera.rotation.x, radians(-89.f), radians(89.f));

    camera.rotation.y -= mouseDiff.x * 0.001f;

    if (isDown(FS::Buttons::BUTTON_SHIFT)) {
        moveSpeed *= 2.f;
    }
    if (isDown(FS::Buttons::BUTTON_W)) {
        Vector3 rotated = rotate(Vector3{ 0.f, 0.f, moveSpeed }, camera.rotation.y, Vector3{ 0.f, 1.f, 0.f });
        camera.position.x += rotated.x;
        camera.position.y += rotated.y;
        camera.position.z += rotated.z;
    }
    if (isDown(FS::Buttons::BUTTON_S)) {
        Vector3 rotated = rotate(Vector3{ 0.f, 0.f, -moveSpeed }, camera.rotation.y, Vector3{ 0.f, 1.f, 0.f });
        camera.position.x += rotated.x;
        camera.position.y += rotated.y;
        camera.position.z += rotated.z;
    }
    if (isDown(FS::Buttons::BUTTON_A)) {
        Vector3 rotated = rotate(Vector3{ -moveSpeed, 0.f, 0.f }, camera.rotation.y, Vector3{ 0.f, 1.f, 0.f });
        camera.position.x += rotated.x;
        camera.position.y += rotated.y;
        camera.position.z += rotated.z;
    }
    if (isDown(FS::Buttons::BUTTON_D)) {
        Vector3 rotated = rotate(Vector3{ moveSpeed, 0.f, 0.f }, camera.rotation.y, Vector3{ 0.f, 1.f, 0.f });
        camera.position.x += rotated.x;
        camera.position.y += rotated.y;
        camera.position.z += rotated.z;
    }

    if (isDown(FS::Buttons::BUTTON_SPACE)) {
        camera.position.y += moveSpeed;
    }
    if (isDown(FS::Buttons::BUTTON_CTRL)) {
        camera.position.y -= moveSpeed;
    }

    float pi          = static_cast<float>(std::numbers::pi);
    float rotateSpeed = 0.05f;
    if (isDown(FS::Buttons::BUTTON_UP)) {
        camera.rotation.x -= rotateSpeed / pi;
        camera.rotation.x = std::clamp(camera.rotation.x, radians(-89.f), radians(89.f));
    }
    if (isDown(FS::Buttons::BUTTON_DOWN)) {
        camera.rotation.x += rotateSpeed / pi;
        camera.rotation.x = std::clamp(camera.rotation.x, radians(-89.f), radians(89.f));
    }
    if (isDown(FS::Buttons::BUTTON_LEFT)) {
        camera.rotation.y += rotateSpeed / pi;
    }
    if (isDown(FS::Buttons::BUTTON_RIGHT)) {
        camera.rotation.y -= rotateSpeed / pi;
    }

    if (isDown(FS::Buttons::BUTTON_Q)) {
        camera.position = { 0.f, 0.f, 0.f };
        camera.rotation = { 0.f, 0.f, 0.f };
    }
}
void handleInput(FS::Window &window, Scene &scene,float dt) {
    FS::Input &input = window.getInput();
    Camera &camera   = scene.camera;

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
    cameraMovementSystem(camera, window,dt);
}
int main() {
    Timer timer;
    Timer initTimer;
    startTimer(initTimer);
    uint32_t zenith     = AssetManager::loadModel("models/Zenith.obj","textures/Zenith.png", true);
    uint32_t bed        = AssetManager::loadModel("models/Bed.obj","textures/white.png");
    uint32_t demonSkull = AssetManager::loadModel("models/DemonSkull.obj","textures/white.png");
    uint32_t sponza     = AssetManager::loadModel("models/sponza.obj","textures/white.png");
    uint32_t cube       = AssetManager::loadModel("models/cube.obj","this_is_invalid");

    FS::Window window("FSEngine", 720, 720);
    window.showCursor(false);

    DeviceContext deviceContext = {};
    TIME_FUNC("initDeviceContext", timer, initDeviceContext(deviceContext, MAX_ENTITIES, window));

    SwapchainContext swapchainContext = {};
    TIME_FUNC("initSwapchainContext", timer, initSwapchainContext(deviceContext, swapchainContext, window));

    VulkanRenderer renderer = {};
    TIME_FUNC("initVulkanRenderer", timer, initVulkanRenderer(deviceContext, swapchainContext, renderer));
    window.focus();

    

    Scene scene{
        .camera{
            .position = { 0.f, 0.f, 0.f },
            .rotation = { 0.f, 0.f, 0.f },
        },
        .entities{
            Entity{
                .modelIndex = cube,
                .transform  = {
                    .position = { 20.f, 0.f, -10.f },
                    .rotation = { 0.f, 0.f, 0.f },
                    .scale    = 3.f,
                },
            },
            Entity{
                .modelIndex = zenith,
                .transform  = {
                    .position = { 20.f, 2.f, 0.f },
                    .rotation = { 0.f, 0.f, 0.f },
                    .scale    = 1.f,
                },
            },
            Entity{
                .modelIndex = bed,
                .transform  = {
                    .position = { 30.f, 2.f, 0.f },
                    .rotation = { 0.f, radians(90.f), 0.f },
                    .scale    = 1.f,
                },
            },
            Entity{
                .modelIndex = demonSkull,
                .transform  = {
                    .position = { 20.f, -200.f * 0.3f + 4.f, 10.f },
                    .rotation = { 0.f, radians(-90.f), 0.f },
                    .scale    = 0.3f,
                },
            },
            Entity{
                .modelIndex = sponza,
                .transform  = {
                    .position = { 0.f, 0.f, 0.f },
                    .rotation = { 0.f, 0.f, 0.f },
                    .scale    = 0.1f,
                },
            },
        }
    };

    scene.entities.resize(MAX_ENTITIES);
    for(uint32_t i=0;i<MAX_ENTITIES;++i){
        const float distance = 3.f;
        const float x        = static_cast<float>(i % static_cast<uint32_t>(sqrt(MAX_ENTITIES))) * distance;
        const float z        = static_cast<float>(i / sqrt(MAX_ENTITIES)) * distance;
        scene.entities[i] = {
            .modelIndex = cube,
            .transform = {
                .position = {x,0.f,z},
                .rotation = {0.f,0.f,0.f},
                .scale = 1.f,
            },
        };
    }
    setScene(deviceContext, renderer, scene);

    endTimer(initTimer);
    LOG_INFO("Initialization : " << microsecToms(initTimer.diff) << " ms");

    float diff = 16.f;
    while (window.isOpen()) {
        startTimer(timer);
        renderScene(deviceContext, swapchainContext, window, renderer, scene);
        handleInput(window, scene,diff);
        endTimer(timer);
        scene.entities[0].transform.rotation.y += radians(90) * (timer.diff / 1000000.f);
        diff = microsecToms(timer.diff);
        LOG_LIVE("FPS: " << microsecToFPS(timer.diff) << " Frame:" << microsecToms(timer.diff) << " ms");
        window.processMessages();
    }

    vkDeviceWaitIdle(deviceContext.device);
    cleanupVulkanRenderer(deviceContext, renderer);
    cleanupSwapchainContext(deviceContext, swapchainContext);
    cleanupDeviceContext(deviceContext);
    window.close();
}