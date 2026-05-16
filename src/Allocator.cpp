#ifdef _WIN32
#include "Allocator.h"
#include <cstdlib>

void* Allocator::allocation(size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    return _aligned_malloc(size, alignment);
}

void* Allocator::reAllocation(void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    return _aligned_realloc(pOriginal, size, alignment);
}

void Allocator::free(void* pMemory) {
    _aligned_free(pMemory);
}

void* Allocator::allocation(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    return static_cast<Allocator*>(pUserData)->allocation(size, alignment, allocationScope);
}

void* Allocator::reAllocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
    return static_cast<Allocator*>(pUserData)->reAllocation(pOriginal, size, alignment, allocationScope);
}

void Allocator::free(void* pUserData, void* pMemory) {
    static_cast<Allocator*>(pUserData)->free(pMemory);
}
#endif
