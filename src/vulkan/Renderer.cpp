#include "Renderer.h"
#include "../AssetManager.h"
#include "../Logging.h"
#include "../Utilities.h"
#include "VulkanUtils.h"
#include <array>
#include <assert.h>
#include <chrono> // IWYU pragma: keep
#include <cstring>
#include <thread>

VkShaderModule createShaderModule(VkDevice &device, const std::vector<char> &code) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0,
        .codeSize = (code.size() * sizeof(char)),
        .pCode    = reinterpret_cast<const uint32_t *>(code.data())
    };
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule);
    return shaderModule;
}
VkResult createPipelineLayout(DeviceContext &deviceContext, GraphicsPipeline &pipeline) {
    const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .setLayoutCount         = 1,
        .pSetLayouts            = &deviceContext.descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = nullptr
    };

    return vkCreatePipelineLayout(deviceContext.device, &pipelineLayoutCreateInfo, nullptr, &pipeline.pipelineLayout);
}
VkVertexInputBindingDescription getBindingDescription() {
    return {
        .binding   = 0,
        .stride    = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}
const std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription() {
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(Vertex, pos) },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(Vertex, normal) },
        VkVertexInputAttributeDescription{
            .location = 2,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32_SFLOAT,
            .offset   = offsetof(Vertex, texCoord) }
    };
}

VkResult createDescriptorSets(DeviceContext &deviceContext, VulkanRenderer &renderer) {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        layouts[i] = deviceContext.descriptorSetLayout;
    }
    VkDescriptorSetAllocateInfo allocateInfo = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = deviceContext.descriptorPool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts        = layouts
    };
    VkResult result = vkAllocateDescriptorSets(deviceContext.device, &allocateInfo, renderer.descriptorSets);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate descriptor sets");
        return result;
    }

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = renderer.cameraUniform.buffer.bufferHandle,
            .offset = 0,
            .range  = sizeof(UniformBufferData)
        };
        VkDescriptorBufferInfo storageBufferInfo = {
            .buffer = renderer.entityBuffer.bufferHandle,
            .offset = 0,
            .range  = sizeof(EntityBufferData) * renderer.entities.size()
        };
        VkDescriptorImageInfo imageInfo[MAX_TEXTURES];
        for (uint32_t i = 0; i < renderer.textures.size(); i++) {
            imageInfo[i] = {
                .sampler     = deviceContext.sampler,
                .imageView   = renderer.textures[i].imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }
        VkWriteDescriptorSet descriptorWrites[3] = {
            {
                .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext            = nullptr,
                .dstSet           = renderer.descriptorSets[i],
                .dstBinding       = 0,
                .dstArrayElement  = 0,
                .descriptorCount  = 1,
                .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo       = nullptr,
                .pBufferInfo      = &bufferInfo,
                .pTexelBufferView = nullptr,
            },
            {
                .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext            = nullptr,
                .dstSet           = renderer.descriptorSets[i],
                .dstBinding       = 1,
                .dstArrayElement  = 0,
                .descriptorCount  = static_cast<uint32_t>(renderer.textures.size()),
                .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo       = imageInfo,
                .pBufferInfo      = nullptr,
                .pTexelBufferView = nullptr,
            },
            {
                .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext            = nullptr,
                .dstSet           = renderer.descriptorSets[i],
                .dstBinding       = 2,
                .dstArrayElement  = 0,
                .descriptorCount  = 1,
                .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pImageInfo       = nullptr,
                .pBufferInfo      = &storageBufferInfo,
                .pTexelBufferView = nullptr,
            },
        };

        vkUpdateDescriptorSets(deviceContext.device, (sizeof(descriptorWrites) / sizeof(descriptorWrites[0])), descriptorWrites, 0, nullptr);
    }

    return result;
}

void createGraphicsPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline, const std::string &shaderPath) {
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceContext.physicalDevice, deviceContext.surface, &surfaceCaps);
    const std::vector<char> code                              = readFile(shaderPath);
    VkShaderModule module                                     = createShaderModule(deviceContext.device, code);
    const VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = 0,
        .stage               = VK_SHADER_STAGE_VERTEX_BIT,
        .module              = module,
        .pName               = "vertMain",
        .pSpecializationInfo = nullptr
    };
    const VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = 0,
        .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module              = module,
        .pName               = "fragMain",
        .pSpecializationInfo = nullptr
    };

    const VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    const VkVertexInputBindingDescription bindingDescription                     = getBindingDescription();
    const std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = getAttributeDescription();
    const VkPipelineVertexInputStateCreateInfo vertexInputState                  = {
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                           = nullptr,
        .flags                           = 0,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions    = attributeDescriptions.data()
    };

    const VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    const VkViewport viewport{
        .x        = 0.f,
        .y        = 0.f,
        .width    = static_cast<float>(surfaceCaps.currentExtent.width),
        .height   = static_cast<float>(surfaceCaps.currentExtent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    const VkRect2D scissor{
        .offset = VkOffset2D{ 0, 0 },
        .extent = surfaceCaps.currentExtent
    };
    const VkPipelineViewportStateCreateInfo viewportState = {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0,
        .viewportCount = 1,
        .pViewports    = &viewport,
        .scissorCount  = 1,
        .pScissors     = &scissor
    };

    const VkPipelineRasterizationStateCreateInfo rasterizerState = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = VK_CULL_MODE_BACK_BIT,
        .frontFace               = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.f,
        .depthBiasClamp          = 0.f,
        .depthBiasSlopeFactor    = 0.f,
        .lineWidth               = 1.f
    };

    const VkPipelineMultisampleStateCreateInfo multisampleState = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = 0,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 0.f,
        .pSampleMask           = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE
    };

    const VkPipelineDepthStencilStateCreateInfo depthStencilState = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = 0,
        .depthTestEnable       = VK_TRUE,
        .depthWriteEnable      = VK_TRUE,
        .depthCompareOp        = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
        .front                 = {},
        .back                  = {},
        .minDepthBounds        = 0.f,
        .maxDepthBounds        = 1.f
    };

    const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable         = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
        .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    const VkPipelineColorBlendStateCreateInfo colorBlendState = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext           = nullptr,
        .flags           = 0,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments    = &colorBlendAttachment,
        .blendConstants  = {}
    };

    const VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    const VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext             = nullptr,
        .flags             = 0,
        .dynamicStateCount = 2,
        .pDynamicStates    = dynamicStates
    };

    const VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext                   = nullptr,
        .viewMask                = 0,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &swapchainContext.surfaceFormat.format,
        .depthAttachmentFormat   = VK_FORMAT_D32_SFLOAT,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    const VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &pipelineRenderingCreateInfo,
        .flags               = 0,
        .stageCount          = 2,
        .pStages             = shaderStages,
        .pVertexInputState   = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pTessellationState  = nullptr,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizerState,
        .pMultisampleState   = &multisampleState,
        .pDepthStencilState  = &depthStencilState,
        .pColorBlendState    = &colorBlendState,
        .pDynamicState       = &dynamicState,
        .layout              = pipeline.pipelineLayout,
        .renderPass          = nullptr,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = 0
    };

    pipeline.pipeline = VK_NULL_HANDLE;
    vkCreateGraphicsPipelines(deviceContext.device, nullptr, 1, &graphicsPipelineInfo, nullptr, &pipeline.pipeline);
    vkDestroyShaderModule(deviceContext.device, module, nullptr);
}
void updateUniformBuffer(const SwapchainContext &swapchainContext, const VulkanRenderer &renderer, const Camera &camera) {
    const float aspectRatio   = static_cast<float>(swapchainContext.extent.width) / static_cast<float>(swapchainContext.extent.height);
    const Vector3 cameraPoint = rotate(Vector3{ 0.f, 0.f, 1.f }, camera.rotation);
    const Vector3 lookatpos   = { camera.position.x + cameraPoint.x, camera.position.y + cameraPoint.y, camera.position.z + cameraPoint.z };
    UniformBufferData uboData = {};
    uboData.view              = lookAt(camera.position, lookatpos, Vector3{ 0.f, 1.f, 0.f });
    uboData.projection        = perspective(radians(60.f), aspectRatio, 0.1f, 10000.f);

    std::memcpy(renderer.cameraUniform.mapped, &uboData, sizeof(uboData));
}
void updateEntityRange(const std::vector<Entity> &entities, EntityBufferData entityData[], const uint32_t start, const uint32_t end) {
    assert(start <= entities.size() && end <= entities.size());
    for (uint32_t i = start; i < end; i++) {
        const uint32_t meshIndex   = entities[i].modelHandle;
        entityData[i].model        = modelMatrix(entities[i].transform);
        entityData[i].textureIndex = meshIndex;
    }
}
void updateEntityBuffer(DeviceContext &deviceContext, Buffer entityBuffer, const std::vector<Entity> &entities) {
    if(entities.empty()){
        return;
    }
    const uint32_t entityCount        = static_cast<uint32_t>(entities.size());
    static const uint32_t threadCount = std::thread::hardware_concurrency();
    const uint32_t size               = static_cast<uint32_t>(entities.size() * sizeof(EntityBufferData));
    if (entityCount < threadCount) {
        std::vector<EntityBufferData> entityData(entityCount);
        updateEntityRange(entities, entityData.data(), 0, entityCount);
        uploadToDeviceBuffer(deviceContext, entityBuffer, entityData.data(), size);
        return;
    }
    static std::vector<std::thread> entityUpdateThreads(threadCount);
    const uint32_t entityPerThread   = static_cast<uint32_t>(entityCount / threadCount);
    const uint32_t remainingEntities = entityCount % threadCount;
    std::vector<EntityBufferData> entityData(entityCount);
    uint32_t start = 0, end = 0;
    for (uint32_t i = 0; i < threadCount; ++i) {
        end                    = start + entityPerThread + (i < remainingEntities ? 1 : 0);
        entityUpdateThreads[i] = std::thread(updateEntityRange, std::cref(entities), entityData.data(), start, end);
        start                  = end;
    }
    for (uint32_t i = 0; i < threadCount; ++i) {
        entityUpdateThreads[i].join();
    }
    uploadToDeviceBuffer(deviceContext, entityBuffer, entityData.data(), size);
}
void recordCommandBuffer(FrameData frameData, SwapchainContext &swapchainContext, VulkanRenderer &renderer, uint32_t imageIndex, uint32_t frameIndex) {
    VkImage image         = swapchainContext.images[imageIndex];
    VkImageView imageView = swapchainContext.imageViews[imageIndex];

    constexpr VkCommandBufferBeginInfo beginInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = 0,
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
        VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    transitionImageLayout(
        image,
        frameData.commandBuffer,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_NONE,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT, 1);

    constexpr VkClearValue clearColor                   = { .color = VkClearColorValue{ { 0.01f, 0.02f, 0.05f, 1.f } } };
    constexpr VkClearValue clearDepth                   = { .depthStencil = VkClearDepthStencilValue{ 1.f, 0 } };
    const VkRenderingAttachmentInfo colorAttachmentInfo = {
        .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext              = nullptr,
        .imageView          = imageView,
        .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode        = VK_RESOLVE_MODE_NONE,
        .resolveImageView   = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue         = clearColor
    };
    const VkRenderingAttachmentInfo depthAttachmentInfo = {
        .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext              = nullptr,
        .imageView          = swapchainContext.depth.imageView,
        .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .resolveMode        = VK_RESOLVE_MODE_NONE,
        .resolveImageView   = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp            = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue         = clearDepth
    };

    const VkRenderingInfo renderingInfo = {
        .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext                = nullptr,
        .flags                = 0,
        .renderArea           = { .offset = { 0, 0 }, .extent = swapchainContext.extent },
        .layerCount           = 1,
        .viewMask             = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &colorAttachmentInfo,
        .pDepthAttachment     = &depthAttachmentInfo,
        .pStencilAttachment   = nullptr
    };
    // Rendering START
    vkCmdBeginRendering(frameData.commandBuffer, &renderingInfo);

    vkCmdBindPipeline(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline.pipeline);
    const VkViewport viewport{
        .x        = 0,
        .y        = 0,
        .width    = static_cast<float>(swapchainContext.extent.width),
        .height   = static_cast<float>(swapchainContext.extent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };
    vkCmdSetViewport(frameData.commandBuffer, 0, 1, &viewport);
    const VkRect2D scissor{
        .offset = { 0, 0 },
        .extent = swapchainContext.extent
    };
    vkCmdSetScissor(frameData.commandBuffer, 0, 1, &scissor);

    if(!renderer.entities.empty()){
        vkCmdBindDescriptorSets(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline.pipelineLayout, 0, 1, &renderer.descriptorSets[frameIndex], 0, nullptr);
    }
    if(renderer.vertexBuffer.bufferHandle != VK_NULL_HANDLE && renderer.indexBuffer.bufferHandle != VK_NULL_HANDLE && renderer.indirectDrawBuffer.bufferHandle != VK_NULL_HANDLE){
        uint32_t entityCount = static_cast<uint32_t>(renderer.entities.size());
        VkDeviceSize vboffset = 0;
        vkCmdBindVertexBuffers(frameData.commandBuffer, 0, 1, &renderer.vertexBuffer.bufferHandle, &vboffset);
        vkCmdBindIndexBuffer(frameData.commandBuffer, renderer.indexBuffer.bufferHandle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexedIndirect(frameData.commandBuffer, renderer.indirectDrawBuffer.bufferHandle, 0, entityCount, sizeof(VkDrawIndexedIndirectCommand));
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
        VK_IMAGE_ASPECT_COLOR_BIT, 1);

    vkEndCommandBuffer(frameData.commandBuffer);
}
void renderScene(DeviceContext &deviceContext, SwapchainContext &swapchainContext, FS::Window &window, VulkanRenderer &renderer,Camera& camera) {
    FS::RenderState &renderState       = window.getRenderState();
    bool windowMinimized               = (renderState.width <= 0) || (renderState.height <= 0);
    uint32_t &frameIndex               = renderer.frameIndex;
    VkFence &drawFence                 = renderer.frames[frameIndex].drawFence;
    VkSemaphore &imageAcquireSemaphore = renderer.frames[frameIndex].imageAcquireSemaphore;

    vkWaitForFences(deviceContext.device, 1, &drawFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex    = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(deviceContext.device, swapchainContext.swapchain, UINT64_MAX, imageAcquireSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || windowMinimized) {
        recreateSwapchain(deviceContext, swapchainContext, window);
        return;
    }
    VkSemaphore renderFinishedSemaphore = renderer.renderFinishedSemaphores[imageIndex];

    vkResetFences(deviceContext.device, 1, &drawFence);

    updateEntityBuffer(deviceContext, renderer.entityBuffer, renderer.scene.entities);
    updateUniformBuffer(swapchainContext, renderer, camera);
    recordCommandBuffer(renderer.frames[frameIndex], swapchainContext, renderer, imageIndex, frameIndex);

    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo               = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &imageAcquireSemaphore,
        .pWaitDstStageMask    = &waitDstStageMask,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &renderer.frames[frameIndex].commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &renderFinishedSemaphore
    };

    VkQueue graphicsQueue = deviceContext.graphicsQueue;
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFence);

    VkPresentInfoKHR presentInfo = {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext              = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &renderFinishedSemaphore,
        .swapchainCount     = 1,
        .pSwapchains        = &swapchainContext.swapchain,
        .pImageIndices      = &imageIndex,
        .pResults           = nullptr
    };
    VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        recreateSwapchain(deviceContext, swapchainContext, window);
    }
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void initPipeline(DeviceContext &deviceContext, SwapchainContext &swapchainContext, GraphicsPipeline &pipeline) {
    createPipelineLayout(deviceContext, pipeline);
    createGraphicsPipeline(deviceContext, swapchainContext, pipeline, "shaders/slang.spv");
}
void cleanupPipeline(DeviceContext &deviceContext, GraphicsPipeline &pipeline) {
    vkDestroyPipeline(deviceContext.device, pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(deviceContext.device, pipeline.pipelineLayout, nullptr);
}
void setScene(DeviceContext &deviceContext, VulkanRenderer &renderer, Scene &scene) {
    if (!renderer.textures.empty()) {
        for (Texture &texture : renderer.textures) {
            cleanupTexture(deviceContext, texture);
        }
        renderer.textures.clear();
    }

    if (!renderer.entities.empty()) {
        vkFreeDescriptorSets(deviceContext.device, deviceContext.descriptorPool, MAX_FRAMES_IN_FLIGHT, renderer.descriptorSets);
        cleanupBuffer(deviceContext, renderer.entityBuffer);
        renderer.entities.clear();
    }
    if (renderer.vertexBuffer.bufferHandle != VK_NULL_HANDLE) {
        cleanupBuffer(deviceContext, renderer.vertexBuffer);
    }
    if (renderer.indexBuffer.bufferHandle != VK_NULL_HANDLE) {
        cleanupBuffer(deviceContext, renderer.indexBuffer);
    }
    renderer.scene = scene;

    std::vector<Entity> &entities = scene.entities;
    assert(entities.size() <= MAX_ENTITIES);
    assert(AssetManager::getModelCount() <= MAX_TEXTURES);

    renderer.textures.resize(MAX_TEXTURES);
    for (uint32_t i = 0; i < MAX_TEXTURES; ++i) {
        const std::string &texturePath = i < AssetManager::getModelCount() ? AssetManager::getModel(i).texturePath : "textures/invalid.png";
        renderer.textures[i]           = createTexture(deviceContext, texturePath);
    }

    renderer.entities.resize(entities.size());
    renderer.entityBuffer = createEntityStorageBuffer(deviceContext, static_cast<uint32_t>(renderer.entities.size()));
    createDescriptorSets(deviceContext, renderer);

    // concatenate meshes into one vertex and index buffer
    uint32_t totalVertex             = 0;
    uint32_t totalIndex              = 0;
    const std::vector<Model> &models = AssetManager::getModels();
    for (const Model &model : models) {
        totalIndex += model.indices.size();
        totalVertex += model.vertices.size();
    }
    std::vector<Vertex> vertices(totalVertex);
    std::vector<uint32_t> indices(totalIndex);
    std::vector<uint32_t> indexOffsets(models.size());
    std::vector<uint32_t> vertexOffsets(models.size());
    {
        uint32_t vertexInd = 0;
        uint32_t indexInd  = 0;
        for (uint32_t i = 0; i < models.size(); i++) {
            const Model &model = models[i];
            std::memcpy(vertices.data() + vertexInd, model.vertices.data(), model.vertices.size() * sizeof(Vertex));
            std::memcpy(indices.data() + indexInd, model.indices.data(), model.indices.size() * sizeof(uint32_t));
            indexOffsets[i]  = indexInd;
            vertexOffsets[i] = vertexInd;
            vertexInd += model.vertices.size();
            indexInd += model.indices.size();
        }
    }
    renderer.vertexBuffer = createVertexBuffer(deviceContext, vertices.data(), static_cast<uint32_t>(vertices.size()));
    renderer.indexBuffer  = createIndexBuffer(deviceContext, indices.data(), static_cast<uint32_t>(indices.size()));

    // build up indirect draw buffer of entities and upload it
    std::vector<VkDrawIndexedIndirectCommand> indirectBuffer(entities.size());
    for (uint32_t i = 0; i < entities.size(); i++) {
        const Entity &entity      = entities[i];
        const uint32_t indexCount = static_cast<uint32_t>(models[entity.modelHandle].indices.size());
        VkDrawIndexedIndirectCommand command{
            .indexCount    = indexCount,
            .instanceCount = 1,
            .firstIndex    = indexOffsets[entity.modelHandle],
            .vertexOffset  = static_cast<int32_t>(vertexOffsets[entity.modelHandle]),
            .firstInstance = i
        };
        indirectBuffer[i] = command;
    }
    uploadToDeviceBuffer(deviceContext, renderer.indirectDrawBuffer, indirectBuffer.data(), indirectBuffer.size() * sizeof(VkDrawIndexedIndirectCommand));

    std::vector<EntityBufferData> entityData(entities.size());
    for (uint32_t i = 0; i < entities.size(); i++) {
        const uint32_t meshIndex       = entities[i].modelHandle;
        entityData[i].model            = modelMatrix(entities[i].transform);
        entityData[i].textureIndex     = meshIndex;
        renderer.entities[i].meshIndex = meshIndex;
    }
    uploadToDeviceBuffer(deviceContext, renderer.entityBuffer, entityData.data(), entityData.size() * sizeof(EntityBufferData));
}
void initVulkanRenderer(DeviceContext &deviceContext, SwapchainContext &swapchainContext, VulkanRenderer &renderer) {
    createSemaphores(deviceContext.device, MAX_SWAPCHAIN_IMAGES, renderer.renderFinishedSemaphores);

    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    createCommandBuffers(deviceContext, MAX_FRAMES_IN_FLIGHT, commandBuffers);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        FrameData &frame    = renderer.frames[i];
        frame.commandBuffer = commandBuffers[i];
        createSemaphore(deviceContext.device, &frame.imageAcquireSemaphore);
        createFence(deviceContext.device, &frame.drawFence, VK_FENCE_CREATE_SIGNALED_BIT);
    }

    initPipeline(deviceContext, swapchainContext, renderer.pipeline);

    renderer.cameraUniform      = createUniformBuffer(deviceContext);
    renderer.indirectDrawBuffer = createIndirectDrawBuffer(deviceContext, MAX_ENTITIES);
}
void cleanupVulkanRenderer(DeviceContext &deviceContext, VulkanRenderer &renderer) {
    if (renderer.indexBuffer.bufferHandle != VK_NULL_HANDLE) {
        cleanupBuffer(deviceContext, renderer.indexBuffer);
    }
    if (renderer.vertexBuffer.bufferHandle != VK_NULL_HANDLE) {
        cleanupBuffer(deviceContext, renderer.vertexBuffer);
    }
    if (renderer.indirectDrawBuffer.bufferHandle != VK_NULL_HANDLE) {
        cleanupBuffer(deviceContext, renderer.indirectDrawBuffer);
    }
    if (renderer.cameraUniform.buffer.bufferHandle != VK_NULL_HANDLE) {
        cleanupBuffer(deviceContext, renderer.cameraUniform.buffer);
        renderer.cameraUniform.mapped = nullptr;
    }
    if (renderer.entityBuffer.bufferHandle != VK_NULL_HANDLE) {
        cleanupBuffer(deviceContext, renderer.entityBuffer);
    }

    cleanupPipeline(deviceContext, renderer.pipeline);

    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        commandBuffers[i] = renderer.frames[i].commandBuffer;
    }
    vkFreeCommandBuffers(deviceContext.device, deviceContext.commandPool, MAX_FRAMES_IN_FLIGHT, commandBuffers);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        FrameData &frame = renderer.frames[i];
        vkDestroyFence(deviceContext.device, frame.drawFence, nullptr);
        vkDestroySemaphore(deviceContext.device, frame.imageAcquireSemaphore, nullptr);
    }

    for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i) {
        vkDestroySemaphore(deviceContext.device, renderer.renderFinishedSemaphores[i], nullptr);
    }

    for (Texture &texture : renderer.textures) {
        cleanupTexture(deviceContext, texture);
    }

    vkFreeDescriptorSets(deviceContext.device, deviceContext.descriptorPool, MAX_FRAMES_IN_FLIGHT, renderer.descriptorSets);
}
