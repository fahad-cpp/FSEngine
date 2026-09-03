#include "Renderer.h"
#include "Utilities.h"

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
VkResult createPipelineLayout(DeviceContext &deviceContext, VulkanPipeline &pipeline) {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
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
std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription() {
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, pos) },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color) }
    };
}
void createGraphicsPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, VulkanPipeline &pipeline, const std::string &shaderPath) {
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
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = getAttributeDescription();
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
        .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
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
        .pDepthStencilState = nullptr,
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
Buffer createBuffer(DeviceContext &deviceContext, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memoryProperty) {
    Buffer buffer = {};
    VkBufferCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };
    vkCreateBuffer(deviceContext.device, &createInfo, nullptr, &buffer.buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(deviceContext.device, buffer.buffer, &memRequirements);

    uint32_t memoryIndex = getMemoryIndex(deviceContext.physicalDevice, memRequirements, memoryProperty);
    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryIndex
    };
    vkAllocateMemory(deviceContext.device, &memoryAllocateInfo, nullptr, &buffer.memory);
    vkBindBufferMemory(deviceContext.device, buffer.buffer, buffer.memory, 0);

    return buffer;
}
void createVertexBuffer(DeviceContext &deviceContext, const Vertex *vertices, uint32_t vertexCount, Buffer &vertexBuffer) {
    std::size_t bufferSize = sizeof(vertices[0]) * vertexCount;
    Buffer stagingBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *data = nullptr;
    vkMapMemory(deviceContext.device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, bufferSize);
    vkUnmapMemory(deviceContext.device, stagingBuffer.memory);

    vertexBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(deviceContext, stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

    vkFreeMemory(deviceContext.device, stagingBuffer.memory, nullptr);
    vkDestroyBuffer(deviceContext.device, stagingBuffer.buffer, nullptr);
}
void createIndexBuffer(DeviceContext &deviceContext, const uint32_t *indices, uint32_t indexCount, Buffer &indexBuffer) {
    std::size_t bufferSize = sizeof(indices[0]) * indexCount;
    Buffer stagingBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *data = nullptr;
    vkMapMemory(deviceContext.device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memcpy(data, indices, bufferSize);
    vkUnmapMemory(deviceContext.device, stagingBuffer.memory);

    indexBuffer = createBuffer(deviceContext, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffer(deviceContext, stagingBuffer.buffer, indexBuffer.buffer, bufferSize);

    vkFreeMemory(deviceContext.device, stagingBuffer.memory, nullptr);
    vkDestroyBuffer(deviceContext.device, stagingBuffer.buffer, nullptr);
}
void recordCommandBuffer(FrameData frameData, SwapchainContext &swapchainContext, VulkanPipeline &pipeline, Mesh &mesh, uint32_t imageIndex) {
    VkImage image = swapchainContext.images[imageIndex];
    VkImageView imageView = swapchainContext.imageViews[imageIndex];

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr
    };
    vkBeginCommandBuffer(frameData.commandBuffer, &beginInfo);

    transitionImageLayout(
        image,
        frameData.commandBuffer,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_NONE,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    VkClearValue clearColor = { VkClearColorValue{ { 0.01f, 0.02f, 0.05f, 1.f } } };
    VkRenderingAttachmentInfo attachmentInfo = {
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

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .flags = 0,
        .renderArea = { .offset = { 0, 0 }, .extent = swapchainContext.extent },
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo,
        .pDepthAttachment = nullptr,
        .pStencilAttachment = nullptr
    };
    // Rendering START
    vkCmdBeginRendering(frameData.commandBuffer, &renderingInfo);

    VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = static_cast<float>(swapchainContext.extent.width),
        .height = static_cast<float>(swapchainContext.extent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };
    VkRect2D scissor{
        .offset = { 0, 0 },
        .extent = swapchainContext.extent
    };
    vkCmdBindPipeline(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
    VkDeviceSize vboffset = 0;
    vkCmdBindVertexBuffers(frameData.commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &vboffset);
    vkCmdBindIndexBuffer(frameData.commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdSetViewport(frameData.commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(frameData.commandBuffer, 0, 1, &scissor);
    vkCmdDraw(frameData.commandBuffer, mesh.vertexCount, 1, 0, 0);
    vkCmdDrawIndexed(frameData.commandBuffer, mesh.indexCount, 1, 0, 0, 0);

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
        VK_ACCESS_2_NONE);

    vkEndCommandBuffer(frameData.commandBuffer);
}
void drawFrame(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window, Renderer &renderer, Mesh &mesh) {

    FS::RenderState &renderState = window.getRenderState();
    bool windowMinimized = (renderState.width == 0) || (renderState.height == 0);
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
    recordCommandBuffer(renderer.frames[frameIndex], swapchainContext, renderer.pipeline, mesh, imageIndex);

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
    VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        recreateSwapchain(deviceContext, swapchainContext, window);
    }
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void initPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, VulkanPipeline &pipeline) {
    createPipelineLayout(deviceContext, pipeline);
    createGraphicsPipeline(deviceContext, swapchainContext, pipeline, "shaders/slang.spv");
}
void cleanupPipeline(DeviceContext &deviceContext, VulkanPipeline &pipeline) {
    vkDestroyPipeline(deviceContext.device, pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(deviceContext.device, pipeline.pipelineLayout, nullptr);
}
void initRenderer(DeviceContext &deviceContext, SwapchainContext &swapchainContext, Renderer &renderer) {
    createSemaphores(deviceContext.device, MAX_SWAPCHAIN_IMAGES, renderer.renderFinishedSemaphores);

    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    createCommandBuffers(deviceContext, MAX_FRAMES_IN_FLIGHT, commandBuffers);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        renderer.frames[i].commandBuffer = commandBuffers[i];
        createSemaphore(deviceContext.device, &renderer.frames[i].imageAcquireSemaphore);
        createFence(deviceContext.device, &renderer.frames[i].drawFence, VK_FENCE_CREATE_SIGNALED_BIT);
    }
    initPipeline(deviceContext, swapchainContext, renderer.pipeline);
}
void cleanupRenderer(DeviceContext &deviceContext, Renderer &renderer) {
    cleanupPipeline(deviceContext, renderer.pipeline);
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        commandBuffers[i] = renderer.frames[i].commandBuffer;
    }

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroyFence(deviceContext.device, renderer.frames[i].drawFence, nullptr);
        vkDestroySemaphore(deviceContext.device, renderer.frames[i].imageAcquireSemaphore, nullptr);
    }
    vkFreeCommandBuffers(deviceContext.device, deviceContext.commandPool, MAX_FRAMES_IN_FLIGHT, commandBuffers);
    for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i) {
        vkDestroySemaphore(deviceContext.device, renderer.renderFinishedSemaphores[i], nullptr);
    }
}
void initMesh(DeviceContext &deviceContext, Mesh &mesh, const Vertex *vertices, uint32_t vertexCount, const uint32_t *indices, uint32_t indexCount) {
    mesh.vertexCount = vertexCount;
    mesh.indexCount = indexCount;
    createVertexBuffer(deviceContext, vertices, mesh.vertexCount, mesh.vertexBuffer);
    createIndexBuffer(deviceContext, indices, mesh.indexCount, mesh.indexBuffer);
}
void cleanupMesh(DeviceContext &deviceContext, Mesh &mesh) {
    vkFreeMemory(deviceContext.device, mesh.vertexBuffer.memory, nullptr);
    vkDestroyBuffer(deviceContext.device, mesh.vertexBuffer.buffer, nullptr);

    vkFreeMemory(deviceContext.device, mesh.indexBuffer.memory, nullptr);
    vkDestroyBuffer(deviceContext.device, mesh.indexBuffer.buffer, nullptr);
}