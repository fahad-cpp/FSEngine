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

the `pUserData` is the same as the one in `VkAllocationCallbacks` structure.

one thing you can do is implement the memory allocator as a C++ class and pass its `this` pointer in pUserData.

the `Allocation` function is responsible for making new allocations, 
the `size` parameter gives the size of the allocation in bytes. the `alignment` parameter gives the alignment in bytes , hooking this to a naive allocator such as `malloc` will work for some time , but then randomly crash in some functions. 

so if you are making your own allocator it must honor the `alignment` parameter. 

the final parameter `allocationScope`, tells your application , what the scope ,or lifetime of the allocation is going to be. it is one of the `VkSystemAllocationScope` values which have the following meanings:

<ul>
<li>

`VK_SYSTEM_ALLOCATION_SCOPE_COMMAND` : allocation will be live only for the duration of the command that provoked the allocation
</li>
<li>

`VK_SYSTEM_ALLOCATION_SCOPE_OBJECT`: means that the allocation is directly associated with a Vulkan Object. this allocation will live atleast until the object is destroyed.
</li>
<li>

`VK_SYSTEM_ALLOCATION_SCOPE_CACHE` : means that the allocation is associated with some form of internal cache or a `VkPipelineCache` object.
</li>
<li>

`VK_SYSTEM_ALLOCATION_SCOPE_DEVICE` : means that the allocation is scoped to the device. this type of allocation is made when vulkan implementation needs memory associated with the device that is not tied to a single object. 
</li>
<li>

`VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE` : means that the allocation is scoped to the instance. this type of allocation is made by layers during early parts of vulkan startup such as by `vkCreateInstance()` or `vkEnumeratePhysicalDevices`
</li>
</ul>