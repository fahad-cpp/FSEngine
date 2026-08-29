#include "Renderer.h"

Renderer::Renderer(VulkanDevice &device, VulkanPhysicalDevice &physicalDevice, VulkanSwapchain &swapchain, BackBufferState &backBufferState, VulkanPipeline &pipeline) : m_swapchain(swapchain),m_device(device) {
    m_backBufferState = backBufferState;
    m_pipeline = pipeline.get();
    m_frameIndex = 0;
    m_queue = device.getQueue(physicalDevice.getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT));
}
Renderer::~Renderer() {
    vkQueueWaitIdle(m_queue);
}
void transitionImageLayout(VkImage image,VkCommandBuffer commandBuffer,VkPipelineStageFlags2 srcStageMask,VkPipelineStageFlags2 dstStageMask,VkAccessFlags2 srcAccessMask,VkAccessFlags2 dstAccessMask,VkImageLayout oldLayout,VkImageLayout newLayout){
    VkImageMemoryBarrier2 imageMemoryBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = VkImageSubresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    VkDependencyInfo dependencyInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,
        .dependencyFlags = 0,
        .memoryBarrierCount = 0,
        .pMemoryBarriers = nullptr,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = nullptr,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &imageMemoryBarrier
    };
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}
void Renderer::renderCommands(uint32_t imageIndex) {
    VkCommandBuffer commandBuffer = m_backBufferState.commandBuffers[m_frameIndex];
    VkExtent2D swapchainExtent = m_swapchain.getExtent();
    VkBuffer vertexBuffer = m_vertexBuffer;
    VkImage image = m_swapchain.getImages()[imageIndex];
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    transitionImageLayout(
        image,
        commandBuffer,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_NONE,VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );

    VkClearValue clearColor = { VkClearColorValue{ { 0.01f, 0.01f, 0.02f, 1.f } } };
    VkRenderingAttachmentInfo colorAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = m_swapchain.getImageViews()[imageIndex],
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
        .renderArea = { { 0, 0 }, swapchainExtent },
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
        .pDepthAttachment = nullptr,
        .pStencilAttachment = nullptr
    };
    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };
    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = swapchainExtent
    };
    // Rendering START
    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, m_indicesCount, 1, 0, 0, 0);

    // Rendering END
    vkCmdEndRendering(commandBuffer);

    transitionImageLayout(
        image,
        commandBuffer,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_2_NONE,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );

    vkEndCommandBuffer(commandBuffer);
}
void Renderer::drawFrame() {
    VkSwapchainKHR swapchain = m_swapchain.get();
    uint32_t imageIndex = 0;
    vkWaitForFences(m_device.get(), 1, &m_backBufferState.frameFences[m_frameIndex], VK_TRUE, UINT64_MAX);
    
    VkResult acquireResult = vkAcquireNextImageKHR(m_device.get(), m_swapchain.get(), UINT64_MAX, m_backBufferState.acquireSemaphores[m_frameIndex], VK_NULL_HANDLE, &imageIndex);
    if(acquireResult == VK_ERROR_OUT_OF_DATE_KHR){
        m_swapchain.recreate();
        return;
    }
    vkResetFences(m_device.get(), 1, &m_backBufferState.frameFences[m_frameIndex]);
    
    renderCommands(imageIndex);

    VkPipelineStageFlags waitDstStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_backBufferState.acquireSemaphores[m_frameIndex],
        .pWaitDstStageMask = &waitDstStageFlags,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_backBufferState.commandBuffers[m_frameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_backBufferState.renderSemaphores[imageIndex]
    };
    vkQueueSubmit(m_queue, 1, &submitInfo, m_backBufferState.frameFences[m_frameIndex]);
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_backBufferState.renderSemaphores[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };
    VkResult presentResult = vkQueuePresentKHR(m_queue, &presentInfo);
    if(presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR){
        m_swapchain.recreate();
    }
    m_frameIndex = (m_frameIndex + 1) % m_framesInFlight;
}

void Renderer::setVertices(VertexBuffer &vertexBuffer) {
    m_verticesCount = vertexBuffer.getVertexCount();
    m_vertexBuffer = vertexBuffer.get();
}
void Renderer::setIndices(IndexBuffer &indexBuffer) {
    m_indicesCount = indexBuffer.getIndexCount();
    m_indexBuffer = indexBuffer.get();
}