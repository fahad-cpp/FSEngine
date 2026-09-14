#include "Renderer.h"
#include "../Utilities.h"
#include "VulkanUtils.h"
#include <array>
#include <chrono> // IWYU pragma: keep
#include <cstring>


VkShaderModule createShaderModule(VkDevice &device, const std::vector<char> &code) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = (code.size() * sizeof(char)),
        .pCode = reinterpret_cast<const uint32_t *>(code.data())
    };
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule);
    return shaderModule;
}
VkResult createDescriptorPool(DeviceContext &deviceContext, GraphicsPipeline &pipeline) {
    VkDescriptorPoolSize poolSizes[2] = {
        { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = MAX_FRAMES_IN_FLIGHT },
        { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = MAX_FRAMES_IN_FLIGHT }
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]),
        .pPoolSizes = poolSizes
    };
    return vkCreateDescriptorPool(deviceContext.device, &descriptorPoolCreateInfo, nullptr, &pipeline.descriptorPool);
}
VkResult createDescriptorSets(DeviceContext &deviceContext, GraphicsPipeline &pipeline, FrameData *frames, Mesh &mesh) {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        layouts[i] = pipeline.descriptorSetLayout;
    }
    VkDescriptorSetAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = pipeline.descriptorPool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts
    };
    VkResult result = vkAllocateDescriptorSets(deviceContext.device, &allocateInfo, pipeline.descriptorSets);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = frames[i].uniformBuffer.buffer,
            .offset = 0,
            .range = sizeof(UniformBufferData)
        };
        VkDescriptorImageInfo imageInfo = {
            .sampler = mesh.texture.sampler,
            .imageView = mesh.texture.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkWriteDescriptorSet descriptorWrites[2] = {
            { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              .pNext = nullptr,
              .dstSet = pipeline.descriptorSets[i],
              .dstBinding = 0,
              .dstArrayElement = 0,
              .descriptorCount = 1,
              .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              .pImageInfo = nullptr,
              .pBufferInfo = &bufferInfo,
              .pTexelBufferView = nullptr },
            { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              .pNext = nullptr,
              .dstSet = pipeline.descriptorSets[i],
              .dstBinding = 1,
              .dstArrayElement = 0,
              .descriptorCount = 1,
              .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              .pImageInfo = &imageInfo,
              .pBufferInfo = nullptr,
              .pTexelBufferView = nullptr }
        };

        vkUpdateDescriptorSets(deviceContext.device, (sizeof(descriptorWrites) / sizeof(descriptorWrites[0])), descriptorWrites, 0, nullptr);
    }

    return result;
}
VkResult createDescriptorSetLayout(DeviceContext &deviceContext, GraphicsPipeline &pipeline) {
    const VkDescriptorSetLayoutBinding descriptorBindings[2] = {
        { .binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
          .pImmutableSamplers = nullptr },
        { .binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
          .pImmutableSamplers = nullptr }
    };
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = sizeof(descriptorBindings) / sizeof(descriptorBindings[0]),
        .pBindings = descriptorBindings
    };
    return vkCreateDescriptorSetLayout(deviceContext.device, &descriptorSetLayoutInfo, nullptr, &pipeline.descriptorSetLayout);
}
VkResult createPipelineLayout(DeviceContext &deviceContext, GraphicsPipeline &pipeline) {
    const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &pipeline.descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    return vkCreatePipelineLayout(deviceContext.device, &pipelineLayoutCreateInfo, nullptr, &pipeline.pipelineLayout);
}
VkVertexInputBindingDescription getBindingDescription() {
    return {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}
std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription() {
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, pos) },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, normal) },
        VkVertexInputAttributeDescription{
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, texCoord) }
    };
}
void createGraphicsPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline, const std::string &shaderPath) {
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceContext.physicalDevice, deviceContext.surface, &surfaceCaps);
    const std::vector<char> code = readFile(shaderPath);
    VkShaderModule module = createShaderModule(deviceContext.device, code);
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = module,
        .pName = "vertMain",
        .pSpecializationInfo = nullptr
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = module,
        .pName = "fragMain",
        .pSpecializationInfo = nullptr
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkVertexInputBindingDescription bindingDescription = getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = getAttributeDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport{
        .x = 0.f,
        .y = 0.f,
        .width = static_cast<float>(surfaceCaps.currentExtent.width),
        .height = static_cast<float>(surfaceCaps.currentExtent.height),
        .minDepth = 0.f,
        .maxDepth = 0.f
    };

    VkRect2D scissor{
        .offset = VkOffset2D{ 0, 0 },
        .extent = surfaceCaps.currentExtent
    };
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterizerState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.f,
        .depthBiasClamp = 0.f,
        .depthBiasSlopeFactor = 0.f,
        .lineWidth = 1.f
    };

    VkPipelineMultisampleStateCreateInfo multisampleState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_TRUE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.f,
        .maxDepthBounds = 1.f
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {}
    };

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates
    };

    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchainContext.surfaceFormat.format,
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipelineRenderingCreateInfo,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pTessellationState = nullptr,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizerState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
        .layout = pipeline.pipelineLayout,
        .renderPass = nullptr,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0
    };

    pipeline.pipeline = VK_NULL_HANDLE;
    vkCreateGraphicsPipelines(deviceContext.device, nullptr, 1, &graphicsPipelineInfo, nullptr, &pipeline.pipeline);
    vkDestroyShaderModule(deviceContext.device, module, nullptr);
}

void updateUniformBuffer(FrameData &frame, SwapchainContext &swapchainContext,Camera& camera) {
#if 0
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
#else
    float time = 4.f;
#endif
    Vector3 cameraPoint = rotate(Vector3{0.f,0.f,1.f},camera.rotation);
    //cameraPoint = normalize(cameraPoint);
    Vector3 lookatpos = {camera.position.x + cameraPoint.x,camera.position.y + cameraPoint.y, camera.position.z + cameraPoint.z};
    float aspectRatio = static_cast<float>(swapchainContext.extent.width) / static_cast<float>(swapchainContext.extent.height);
    UniformBufferData uboData = {};
    uboData.model = rotate(unitMatrix4(1.f), time * radians(90), Vector3{ 0.f, 1.f, 0.f });
    uboData.view = lookAt(camera.position, lookatpos, Vector3{ 0.f, 1.f, 0.f });
    uboData.projection = perspective(radians(45.f), aspectRatio, 0.1f, 10000.f);
    uboData.projection.values[1][1] *= -1;

    std::memcpy(frame.uniformBufferMapping, &uboData, sizeof(uboData));
}
void recordCommandBuffer(FrameData frameData, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline, Mesh &mesh, uint32_t imageIndex, uint32_t frameIndex) {
    VkImage image = swapchainContext.images[imageIndex];
    VkImageView imageView = swapchainContext.imageViews[imageIndex];

    static const VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr
    };
    vkBeginCommandBuffer(frameData.commandBuffer, &beginInfo);

    transitionImageLayout(
        swapchainContext.depth.image.image,
        frameData.commandBuffer,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT,1);

    transitionImageLayout(
        image,
        frameData.commandBuffer,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_NONE,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,1);

    static const VkClearValue clearColor = { .color = VkClearColorValue{ { 0.01f, 0.02f, 0.05f, 1.f } } };
    static const VkClearValue clearDepth = { .depthStencil = VkClearDepthStencilValue{ 1.f, 0 } };
    VkRenderingAttachmentInfo colorAttachmentInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .resolveImageView = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clearColor
    };
    VkRenderingAttachmentInfo depthAttachmentInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = swapchainContext.depth.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .resolveImageView = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue = clearDepth
    };

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .flags = 0,
        .renderArea = { .offset = { 0, 0 }, .extent = swapchainContext.extent },
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentInfo,
        .pDepthAttachment = &depthAttachmentInfo,
        .pStencilAttachment = nullptr
    };
    // Rendering START
    vkCmdBeginRendering(frameData.commandBuffer, &renderingInfo);

    vkCmdBindPipeline(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

    VkDeviceSize vboffset = 0;
    if (mesh.vertexBuffer.buffer != VK_NULL_HANDLE) {
        vkCmdBindVertexBuffers(frameData.commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &vboffset);
    }

    const VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = static_cast<float>(swapchainContext.extent.width),
        .height = static_cast<float>(swapchainContext.extent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };
    vkCmdSetViewport(frameData.commandBuffer, 0, 1, &viewport);

    const VkRect2D scissor{
        .offset = { 0, 0 },
        .extent = swapchainContext.extent
    };
    vkCmdSetScissor(frameData.commandBuffer, 0, 1, &scissor);
    vkCmdBindDescriptorSets(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &pipeline.descriptorSets[frameIndex], 0, nullptr);
    if (mesh.indexBuffer.buffer != VK_NULL_HANDLE) {
        vkCmdBindIndexBuffer(frameData.commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(frameData.commandBuffer, mesh.indexCount, 1, 0, 0, 0);
    }

    // Rendering END
    vkCmdEndRendering(frameData.commandBuffer);

    transitionImageLayout(
        image,
        frameData.commandBuffer,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_2_NONE,
        VK_IMAGE_ASPECT_COLOR_BIT,1);

    vkEndCommandBuffer(frameData.commandBuffer);
}
void drawFrame(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window, VulkanRenderer &renderer, Mesh &mesh,Camera& camera) {

    FS::RenderState &renderState = window.getRenderState();
    bool windowMinimized = (renderState.width <= 0) || (renderState.height <= 0);
    uint32_t frameIndex = renderer.frameIndex;
    VkFence drawFence = renderer.frames[frameIndex].drawFence;
    VkSemaphore imageAcquireSemaphore = renderer.frames[frameIndex].imageAcquireSemaphore;
    vkWaitForFences(deviceContext.device, 1, &drawFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(deviceContext.device, swapchainContext.swapchain, UINT64_MAX, imageAcquireSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || windowMinimized) {
        recreateSwapchain(deviceContext, swapchainContext, window);
        return;
    }
    VkSemaphore renderFinishedSemaphore = renderer.renderFinishedSemaphores[imageIndex];

    vkResetFences(deviceContext.device, 1, &drawFence);
    recordCommandBuffer(renderer.frames[frameIndex], swapchainContext, renderer.pipeline, mesh, imageIndex, frameIndex);

    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &imageAcquireSemaphore,
        .pWaitDstStageMask = &waitDstStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &renderer.frames[frameIndex].commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderFinishedSemaphore
    };

    VkQueue graphicsQueue = deviceContext.graphicsQueue;
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFence);

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchainContext.swapchain,
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };
    updateUniformBuffer(renderer.frames[frameIndex], swapchainContext,camera);
    VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        recreateSwapchain(deviceContext, swapchainContext, window);
    }
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void initPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline) {
    createDescriptorSetLayout(deviceContext, pipeline);
    createPipelineLayout(deviceContext, pipeline);
    createGraphicsPipeline(deviceContext, swapchainContext, pipeline, "shaders/slang.spv");
}
void cleanupPipeline(DeviceContext &deviceContext, GraphicsPipeline &pipeline) {
    vkDestroyDescriptorSetLayout(deviceContext.device, pipeline.descriptorSetLayout, nullptr);
    vkDestroyPipeline(deviceContext.device, pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(deviceContext.device, pipeline.pipelineLayout, nullptr);
}
void initVulkanRenderer(DeviceContext &deviceContext, SwapchainContext &swapchainContext, VulkanRenderer &renderer, Mesh &mesh) {

    createSemaphores(deviceContext.device, MAX_SWAPCHAIN_IMAGES, renderer.renderFinishedSemaphores);

    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    createCommandBuffers(deviceContext, MAX_FRAMES_IN_FLIGHT, commandBuffers);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {

        FrameData &frame = renderer.frames[i];

        frame.uniformBuffer = createUniformBuffer(deviceContext, &frame.uniformBufferMapping);

        frame.commandBuffer = commandBuffers[i];

        createSemaphore(deviceContext.device, &frame.imageAcquireSemaphore);

        createFence(deviceContext.device, &frame.drawFence, VK_FENCE_CREATE_SIGNALED_BIT);
    }
    initPipeline(deviceContext, swapchainContext, renderer.pipeline);
    createDescriptorPool(deviceContext, renderer.pipeline);
    createDescriptorSets(deviceContext, renderer.pipeline, renderer.frames, mesh);
}
void cleanupVulkanRenderer(DeviceContext &deviceContext, VulkanRenderer &renderer) {

    vkDestroyDescriptorPool(deviceContext.device, renderer.pipeline.descriptorPool, nullptr);
    cleanupPipeline(deviceContext, renderer.pipeline);

    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        commandBuffers[i] = renderer.frames[i].commandBuffer;
    }

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        FrameData &frame = renderer.frames[i];
        vkUnmapMemory(deviceContext.device, frame.uniformBuffer.memory);
        frame.uniformBufferMapping = nullptr;
        cleanupBuffer(deviceContext, frame.uniformBuffer);
        vkDestroyFence(deviceContext.device, frame.drawFence, nullptr);
        vkDestroySemaphore(deviceContext.device, frame.imageAcquireSemaphore, nullptr);
    }

    vkFreeCommandBuffers(deviceContext.device, deviceContext.commandPool, MAX_FRAMES_IN_FLIGHT, commandBuffers);

    for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i) {
        vkDestroySemaphore(deviceContext.device, renderer.renderFinishedSemaphores[i], nullptr);
    }
}
