# Chapter 2 Memory and Resources

Memory is fundamental to operation of any computing system, including vulkan.
in vulkan there are 2 types of memory:<br>
-Host memory<br>
-Device Memory

All resources upon which vulkan operates must be backed by device memory, and it is the application's responsibility to manage this memory.

Further, memory is used to store data structures on the host, vulkan provides a way for you to manage this memory as well.

## Host Memory Management
Whenever vulkan creates new objects it needs memory to store data related to them.

For this it uses Host Memory (RAM).<br>
But vulkan requires allocation to be aligned , because high performance CPU instructions work best on aligned memory addresses.<br>

that is why vullkan implementations will use advanced allocators , but vulkan also provides your application to replace the allocator for certain or almost all operations.<br>

this is performed through `pAllocator` parameter available in most device creation functions

the `pAllocator` parameter is a pointer to `VkAllocationCallbacks` structure, until now we've been setting it to `nullptr` which tells vulkan to use its own internal allocator.

the `VkAllocationCallbacks` structure encapsulates a custom memory allocator that we can provide. the definition of the structure is:

```cpp
typedef struct VkAllocationCallbacks {
    void*                                   pUserData;
    PFN_vkAllocationFunction                pfnAllocation;
    PFN_vkRealocationFunction               pfnReallocation;
    PFN_vkFreeFunction                      pfnFree;
    PFN_vkInternalAllocationNotification    pfnInternalAllocation;
    PFN_vkInternalFreeNotification          pfnInternalFree;
} VkAllocationCallbacks;
```

its essentially a set of function pointers and additional void pointer `pUserData`, the pointer for your application's use, it can point anywhere , vulkan will not dereference it, it can be anything that fit insides a pointer-size blob.

`pfnAllocation` , `pfnReallocation` , `pfnFree` are used for normal object level memory management, they are defined as pointers to functions that match the following declarations:

```cpp
void* VKAPI_CALL Allocation(
    void*                   pUserData,
    size_t                  size,
    size_t                  alignment,
    VkSystemAllocationScope allocationScope);

void* VKAPI_CALL Reallocation(
    void*                   pUserData,
    void*                   pOriginal,
    size_t                  size,
    size_t                  alignment,
    VkSystemAllocationScope allocationScope);

void* VKAPI_CALL Free(
    void* pUserData,
    void* pMemory);
```

