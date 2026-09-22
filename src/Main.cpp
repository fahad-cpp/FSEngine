#include "Timer.h"
#include "vulkan/Renderer.h"
#include <algorithm>
#include <numbers>


/*
    TODO:
        -Find out why demonskull model has weird vertices
*/

void handleInput(FS::Window &window, Scene &scene) {
    FS::Input &input  = window.getInput();
    Camera    &camera = scene.camera;

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }

    float moveSpeed = 0.1f;
    if (isDown(FS::Buttons::BUTTON_SHIFT)) {
        moveSpeed *= 2.f;
    }
    if (isDown(FS::Buttons::BUTTON_W)) {
        Vector3 rotated = rotate(Vector3{ 0.f, 0.f, -moveSpeed }, camera.rotation.y, Vector3{ 0.f, 1.f, 0.f });
        camera.position.x += rotated.x;
        camera.position.y += rotated.y;
        camera.position.z += rotated.z;
    }
    if (isDown(FS::Buttons::BUTTON_S)) {
        Vector3 rotated = rotate(Vector3{ 0.f, 0.f, moveSpeed }, camera.rotation.y, Vector3{ 0.f, 1.f, 0.f });
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
        camera.rotation.x += rotateSpeed / pi;
        camera.rotation.x = std::clamp(camera.rotation.x, radians(-89.f), radians(89.f));
    }
    if (isDown(FS::Buttons::BUTTON_DOWN)) {
        camera.rotation.x -= rotateSpeed / pi;
        camera.rotation.x = std::clamp(camera.rotation.x, radians(-89.f), radians(89.f));
    }
    if (isDown(FS::Buttons::BUTTON_LEFT)) {
        camera.rotation.y -= rotateSpeed / pi;
    }
    if (isDown(FS::Buttons::BUTTON_RIGHT)) {
        camera.rotation.y += rotateSpeed / pi;
    }
}
int main() {
    Timer      timer;
    OBJModel   zenith = loadOBJ("models/Zenith.obj", true);
    OBJModel   bed = loadOBJ("models/Bed.obj");
    OBJModel   demonSkull = loadOBJ("models/DemonSkull.obj");
    FS::Window window("FSEngine", 720, 720);

    DeviceContext deviceContext = {};
    TIME_FUNC("initDeviceContext", timer, initDeviceContext(deviceContext, MAX_ENTITIES, window));

    SwapchainContext swapchainContext = {};
    TIME_FUNC("initSwapchainContext", timer, initSwapchainContext(deviceContext, swapchainContext, window));

    Mesh zenithMesh = {};
    TIME_FUNC("createMesh", timer, zenithMesh = createMesh(deviceContext, zenith, "textures/Zenith.png"));

    Mesh bedMesh = {};
    TIME_FUNC("createMesh", timer, bedMesh = createMesh(deviceContext, bed, "textures/white.png"));

    Mesh demonSkullMesh = {};
    TIME_FUNC("createMesh", timer, demonSkullMesh = createMesh(deviceContext, demonSkull, "textures/white.png"));

    VulkanRenderer renderer = {};
    TIME_FUNC("initVulkanRenderer", timer, initVulkanRenderer(deviceContext, swapchainContext, renderer));
    window.focus();

    Scene scene{
        .camera{
            .position = { 0.f, 3.f, 15.f },
            .rotation = { 0.f, 0.f, 0.f },
        },
        .entities{
            // Entity{
            //     .mesh                 = zenithMesh,
            //     .position             = { 0.f, 0.f, 0.f },
            //     .rotation             = { 0.f, 0.f, 0.f },
            //     .scale                = 1.f,
            //     .uniformBuffer        = {},
            //     .uniformBufferMapping = nullptr,
            //     .descriptorSets       = {},
            // },
            // Entity{
            //     .mesh                 = bedMesh,
            //     .position             = { 0.f, 0.f, 0.f },
            //     .rotation             = { 0.f, 0.f, 0.f },
            //     .scale                = 1.f,
            //     .uniformBuffer        = {},
            //     .uniformBufferMapping = nullptr,
            //     .descriptorSets       = {},
            // },
            Entity{
                .mesh                 = demonSkullMesh,
                .position             = { 0.f, 0.f, 0.f },
                .rotation             = { 0.f, 0.f, 0.f },
                .scale                = 1.f,
                .uniformBuffer        = {},
                .uniformBufferMapping = nullptr,
                .descriptorSets       = {},
            },
        }
    };
    TIME_FUNC("initScene", timer, initScene(deviceContext, scene));

    while (window.isOpen()) {
        startTimer(timer);
        renderScene(deviceContext, swapchainContext, window, renderer, scene);
        handleInput(window, scene);
        window.processMessages();
        endTimer(timer);
        //scene.entities[0].rotation.y += radians(90) * (timer.diff / 1000000.f);
        LOG_LIVE("FPS: " << microsecToFPS(timer.diff));
    }

    vkDeviceWaitIdle(deviceContext.device);
    cleanupScene(deviceContext, scene);
    cleanupMesh(deviceContext, zenithMesh);
    cleanupMesh(deviceContext, bedMesh);
    cleanupMesh(deviceContext, demonSkullMesh);
    cleanupVulkanRenderer(deviceContext, renderer);
    cleanupSwapchainContext(deviceContext, swapchainContext);
    cleanupDeviceContext(deviceContext);
    window.close();
}