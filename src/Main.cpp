#include "AssetManager.h"
#include "ImGuiLayer.h"
#include "Timer.h"
#include "vulkan/Renderer.h"
#include <algorithm>
#include <numbers>

/*
    TODO(fahad-cpp):
    -Restructure code according to Design.md
    -Find a better way to update dirty entities
    -MSAA
    -ECS
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
    FS::Vector2 centerPos               = { windowPos.x + std::floor(static_cast<float>(renderState.width) / 2.f), (windowPos.y + std::floor(static_cast<float>(renderState.height)) / 2.f) };
    FS::Vector2 diff                    = mousePos - centerPos;
    window.setCursorPos(static_cast<uint32_t>(centerPos.x), static_cast<uint32_t>(centerPos.y));
    return { diff.x, diff.y };
}
void cameraMovementSystem(Camera &camera, FS::Window &window, bool mouseLocked, float dt) {
    float moveSpeed   = 0.04f * dt;
    FS::Input &input  = window.getInput();
    Vector2 mouseDiff = ((!mouseLocked) ? Vector2{ 0, 0 } : getMouseDiff(window));

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
void handleInput(FS::Window &window, Camera &camera, float dt) {
#define pressed(button) \
    (input.buttons[button].isDown && input.buttons[button].changed)

    FS::Input &input        = window.getInput();
    static bool mouseLocked = true;

    if (isDown(FS::Buttons::BUTTON_ESC)) {
        window.close();
    }
    if (pressed(FS::Buttons::BUTTON_G)) {
        mouseLocked = !mouseLocked;
        window.showCursor(!mouseLocked);
    }
    cameraMovementSystem(camera, window, mouseLocked, dt);
}
void imguiScenePanel(Scene &scene, float diff) {
    // keep frame times in static ring buffer and initialize linear imgui buffer with it
    constexpr uint32_t FRAMEGRAPH_SIZE = 1024;
    static float ringBuffer[FRAMEGRAPH_SIZE]{ 0.f };
    static float imguiFrames[FRAMEGRAPH_SIZE]{ 0.f };
    static uint32_t ringIndex = 0;
    
    std::fill_n(imguiFrames,FRAMEGRAPH_SIZE,0.f);
    ringBuffer[ringIndex] = diff;
    ringIndex             = ++ringIndex % FRAMEGRAPH_SIZE;

    uint32_t index = ringIndex + 1;
    for (uint32_t i = 0; i < FRAMEGRAPH_SIZE; ++i) {
        imguiFrames[i] = ringBuffer[index];
        index          = ++index % FRAMEGRAPH_SIZE;
    }

    ImGui::Begin("FSEngine");
    float scale = FLT_MAX;
    ImGui::PlotLines(" ", imguiFrames, FRAMEGRAPH_SIZE,0,"FRAMETIME",scale,scale,ImVec2(200,100));
    ImGui::Text("%.2f ms",diff);
    ImGui::Text("%.2f FPS",(1000.f / diff));
    if (ImGui::TreeNode("Entities")) {
        for (uint32_t i = 0; i < scene.entities.size(); i++) {
            const std::string label = "Entity " + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {
                if (ImGui::TreeNode("Transform")) {
                    Transform &transform = scene.entities[i].transform;
                    ImGui::DragFloat3("position", &transform.position.x, 1.f / 10.f, -100.f, 100.f);
                    ImGui::DragFloat3("rotation", &transform.rotation.x, 1.f / radians(360), radians(0), radians(360));
                    ImGui::DragFloat("scale", &transform.scale, 1.f / 100.f, 0.f, 100.f);

                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Lights")) {
        for (uint32_t i = 0; i < scene.lights.size(); ++i) {
            const std::string label = "Light " + std::to_string(i);
            if (ImGui::TreeNode(label.c_str())) {
                Light &light = scene.lights[i];
                ImGui::DragFloat3("Vec", &light.vec.x, 1.f / 10.f, -100.f, 100.f);
                ImGui::DragFloat("radius", &light.radius, 1.f / 10.f, 0.f, 100.f);
                ImGui::DragFloat("illumination", &light.illum, 1.f / 100.f, 0.f, 1.f);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
    ImGui::End();
}
int main() {
    Timer timer;
    Timer initTimer;

    startTimer(initTimer);

    const ModelHandle bed        = AssetManager::loadModel("models/Bed.obj", "textures/white.png");
    const ModelHandle cube       = AssetManager::loadModel("models/cube.obj", "textures/statue.jpg", true);
    const ModelHandle demonSkull = AssetManager::loadModel("models/DemonSkull.obj", "textures/white.png");
    const ModelHandle king       = AssetManager::loadModel("models/king.obj", "textures/invalid.png");
    const ModelHandle sponza     = AssetManager::loadModel("models/sponza.obj", "textures/white.png");
    const ModelHandle vikingRoom = AssetManager::loadModel("models/viking_room.obj", "textures/viking_room.png", true);
    const ModelHandle zenith     = AssetManager::loadModel("models/Zenith.obj", "textures/Zenith.png", true);

    FS::Window window("FSEngine", 1600, 900);
    window.showCursor(false);

    DeviceContext deviceContext = {};
    TIME_FUNC("initDeviceContext", timer, initDeviceContext(deviceContext, window));

    SwapchainContext swapchainContext = {};
    TIME_FUNC("initSwapchainContext", timer, initSwapchainContext(deviceContext, swapchainContext, window));

    VulkanRenderer renderer = {};
    TIME_FUNC("initVulkanRenderer", timer, initVulkanRenderer(deviceContext, swapchainContext, renderer));
    window.focus();

    Camera camera{
        .position = { 0.f, 10.f, 0.f },
        .rotation = { 0.f, radians(-90), 0.f },
    };
    Scene scene{
        .entities{
            Entity{
                .modelHandle = cube,
                .transform   = {
                    .position = { 20.f, 0.f, -10.f },
                    .rotation = { 0.f, 0.f, 0.f },
                    .scale    = 3.f,
                },
            },
            Entity{
                .modelHandle = zenith,
                .transform   = {
                    .position = { 20.f, 2.f, 0.f },
                    .rotation = { 0.f, 0.f, 0.f },
                    .scale    = 1.f,
                },
            },
            Entity{
                .modelHandle = bed,
                .transform   = {
                    .position = { 30.f, 2.f, 0.f },
                    .rotation = { 0.f, radians(90.f), 0.f },
                    .scale    = 1.f,
                },
            },
            Entity{
                .modelHandle = demonSkull,
                .transform   = {
                    .position = { 20.f, -200.f * 0.3f + 4.f, 10.f },
                    .rotation = { 0.f, radians(-90.f), 0.f },
                    .scale    = 0.3f,
                },
            },
            Entity{
                .modelHandle = king,
                .transform   = {
                    .position = { 30.f, 0.f, -10.f },
                    .rotation = { 0.f, 0.f, 0.f },
                    .scale    = 3.f,
                },
            },
            Entity{
                .modelHandle = vikingRoom,
                .transform   = {
                    .position = { 30.f, 0.f, 10.f },
                    .rotation = { 0.f, radians(90), 0.f },
                    .scale    = 5.f,
                },
            },
            Entity{
                .modelHandle = sponza,
                .transform   = {
                    .position = { 0.f, 0.f, 0.f },
                    .rotation = { 0.f, 0.f, 0.f },
                    .scale    = 0.1f,
                },
            },
        },
        .lights = {
            Light{
                .vec    = { -1, -1, 1 },
                .radius = -1.f,
                .illum  = 0.3f,
                .type   = LightType::LT_DIRECTIONAL,
            },
            Light{
                .vec    = { 0, 1, 0 },
                .radius = 40.f,
                .illum  = 0.4f,
                .type   = LightType::LT_POINT,
            },
        }
    };

    // uint32_t cubeCount = MAX_ENTITIES;
    // scene.entities.resize(cubeCount);
    // const uint32_t side  = static_cast<uint32_t>(cbrt(cubeCount));
    // const float distance = 3.f;
    // for (uint32_t i = 0; i < side; ++i) {
    //     for (uint32_t j = 0; j < side; ++j) {
    //         for (uint32_t k = 0; k < side; ++k) {
    //             const float x         = static_cast<float>(k) * distance;
    //             const float z         = static_cast<float>(j) * distance;
    //             const float y         = static_cast<float>(i) * distance;
    //             uint32_t index        = k + (j * side) + (i * side * side);
    //             scene.entities[index] = {
    //                 .modelHandle = cube,
    //                 .transform   = {
    //                     .position = { x, y, z },
    //                     .rotation = { 0.f, 0.f, 0.f },
    //                     .scale    = 1.f,
    //                 },
    //             };
    //         }
    //     }
    // }

    setScene(deviceContext, renderer, scene);

    ImGuiLayer imgui;
    imgui.init(deviceContext, swapchainContext);

    endTimer(initTimer);
    LOG_INFO("Initialization : " << microsecToms(initTimer.diff) << " ms");

    float diff = 16.f;
    while (window.isOpen()) {
        startTimer(timer);
        imgui.handleInput(window);
        imgui.beginFrame();
        imguiScenePanel(scene, diff);

        renderScene(deviceContext, swapchainContext, window, renderer, imgui, camera, scene);
        handleInput(window, camera, diff);
        window.processMessages();
        endTimer(timer);
        scene.entities[0].transform.rotation.y += radians(90) * (timer.diff / 1000000.f);
        diff = microsecToms(timer.diff);
        imgui.endFrame();
        LOG_LIVE("FPS: " << microsecToFPS(timer.diff) << " Frame:" << microsecToms(timer.diff) << " ms");
    }

    vkDeviceWaitIdle(deviceContext.device);
    imgui.shutdown();
    cleanupVulkanRenderer(deviceContext, renderer);
    cleanupSwapchainContext(deviceContext, swapchainContext);
    cleanupDeviceContext(deviceContext);
    window.close();
}