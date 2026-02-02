#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <vulkan/vulkan.h>
#ifdef _WIN32
class Allocator {
public:
    inline operator VkAllocationCallbacks() const {
        VkAllocationCallbacks vulkanAllocator;

        vulkanAllocator.pUserData = (void*)this;
        vulkanAllocator.pfnAllocation = &allocation;
        vulkanAllocator.pfnReallocation = &reAllocation;
        vulkanAllocator.pfnFree = &free;

        vulkanAllocator.pfnInternalAllocation = nullptr;
        vulkanAllocator.pfnInternalFree = nullptr;

        return vulkanAllocator;
    }

    static void* VKAPI_CALL allocation(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationscope);
    static void* VKAPI_CALL reAllocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
    static void VKAPI_CALL free(void* pUserData, void* pMemory);

    void* allocation(size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
    void* reAllocation(void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
    void free(void* pMemory);
};
#endif

#endif