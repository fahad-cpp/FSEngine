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

then there is `pfnInternalAllocation` and `pfnInternalFree`: these functions are used only for notification so your application can keep track of how much memory vulkan is using. These functions have the same signature as `pfnAllocation` and `pfnFree` except that `pfnInternalAllocation` does not return a value, and `pfnInternalFree` shouldn't actually free memory.

```cpp
void VKAPI_CALL pfnInternalAllocationNotification (
    void*                       pUserData,
    size_t                      size, 
    VkInternalAllocationType    allocationType,
    VkSystemAllocationScope     allocationScope
)

void VKAPI_CALL pfnInternalFreeNotification (
    void*                       pUserData
    size_t                      size,
    VkInternalAllocationType    allocationType,
    VkSystemAllocationScope     allocationScope
)
```

if you supply one function you must supply both , if you dont want to supply these functions you can pass `nullptr` to both functions.

Listing 2.1 and Listing 2.2 added to [PracticalNotes](PracticalNotes.md)

## Resources

Vulkan operates on data which is stored in resources which are backed by memory

The fundamental types of resources in vulkan:<br>
- buffers
- images

A **buffer** is a simple linear chunk of data , that can be used for almost anything , data structures, raw arrays, or even image data.

**Images** are structured and have type and format information , can be multidimensional , form arrays of their own , and support advanced operations for reading and writing data to them.

Both types of resources are constructed in two steps , first the resource itself is created , then the resource needs to be backed by memory. the reason for this is to allow application to manage memory itself. Memory management is complex and it is difficult for a driver to get it right all the time. What works well for one application might not work for the other application. therefore it is expected that applications can do a better job of managing memory than drivers can.

For example: an application that that uses a small amount of very large resources and keeps them around for a long time might use one strategy in its memory allocator , while another application that continuously creates and destroys resources might implement another. 

## Buffers 

Buffers are the simplest type of resources but have a wide variety of usages in Vulkan. They are used to store linear structured or unstructured data. which can have a format or be raw in bytes.

to create a new buffer we have to call `vkCreateBuffer()` , the prototype of which is:

```cpp
VkResult vkCreateBuffer(
    VkDevice                        device,
    const VkBufferCreateInfo*       pCreateInfo,
    const VkAllocationCallbacks*    pAllocationCallbacks,
    VkBuffer*                       pBuffer
)
```
as with most parameters in vulkan that need more parameters , they are bundled up in a structure and passed to Vulkan as a pointer. Here , the `pCreateInfo` parameter is a pointer to `VkBufferCreateInfo` structure. the definition of which is:
```cpp
typedef struct VkBufferCreateInfo{
    VkStructureType sType;
    const void* pNext;
    VkBufferCreateFlags flags;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkSharingMode sharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
}VkBufferCreateInfo;
```

*`sType`* = `VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO`

*`pNext`* = should be set to nullptr unless you are using extensions

*`flags`* = Gives vulkan information about properties of the new buffer, in vulkan 1.0 only field defined are related to sparse buffers. for now flags can be set to 0 , the flags will be covered in this chapter later

*`size`* = specifies the size of the buffer in bytes.

*`usage`* = tells vulkan how you are going to use the buffer. it is a bitfield 
made up of combinations of `VkBufferUsageFlagBits` enum , on some architectures , the intended usage of the buffer can have an effect on how it's created. the currently defined bits along with the sections where we'll discuss them are as follows:

- `VK_BUFFER_USAGE_TRANSFER_SRC_BIT` and `VK_BUFFER_USAGE_TRANSFER_DST_BIT` :
Means that the buffer can be source or destination, respectively of a transfer commands. Transfer operations are operations that copy data from a source to a destination they are convered in Chapter 4 "Moving Data"

- `VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT` and `VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT` means that the buffer can be used to back a uniform or a storage texel buffer , respectively. texel buffers are formatted arrays of texels that can be used as the source or destination (in the case of storage buffers) of reads and writes by shaders running on the device. Texel buffers are covered in "Chapter 6 : Shaders and Pipelines".

- `VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT` and `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT` means that the buffer can be used to back uniform or a storage buffers, respectively. as opposed to texel buffers , regular uniform and storage buffers dont have a format associated with them , and therefore can be used to store arbitrary data and data structures. they are covered in "Chapter 6 : Storage and Pipelines".

- `VK_BUFFER_USAGE_INDEX_BUFFER_BIT` and `VK_BUFFER_USAGE_VERTEX_BUFFER_BIT` means that the buffer can be used to store index or vertex data respectively , used in drawing commands. drawing commands including indexed drawing commands in "Chapter 8 : Drawing"

- `VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT` means that the buffer can be used to store parameters used in indirect dispatch and drawing commands , which are commands that take their parameters directly from buffers rather than from your program. These are covered in "Chapter 6 : Shaders and Pipelines" and "Chapter 8 : Drawing"

*`sharingMode`* = indicates how the buffer will behave parallely on multiple command queues supported by the device. because vulkan can execute commands in parallel it needs to know if the buffer will be used by single command at a time or multiple commands in parallel. setting *`sharingMode`* to `VK_SHARING_MODE_EXLUSIVE` says that buffer will only be used on a single queue , whereas setting *`sharingMode`* to `VK_SHARING_MODE_CONCURRENT` indicates that you plan to use the buffer on multiple queues at the same time.
using `VK_SHARING_MODE_CONCURRENT` might result in lower performance on some systems , so unless you need this , set *`sharingMode`* to `VK_SHARING_MODE_EXCLUSINVE` , if you do set *`sharingMode`* to `VK_SHARING_MODE_CONCURRENT` you need to tell vulkan which queues you are going to use the buffer on. you can do this using the *`pQueueFamilyIndices`* member of the `VkBufferCreateInfo`

*`queueFamilyIndexCount`* = size of the *`pQueueFamilyIndices`* array , number of the queue families that the buffer will be based on.

*`pQueueFamilyIndices`* = pointer to an array of queue families that the buffer/resource will be used with.

when *`sharingMode`* is set to `VK_SHARING_MODE_EXCLUSIVE` *`pQueueFamilyIndices`* and *`queueFamilyIndexCount`* both are ignored

Listing 2.3 added to [PracticalNotes](PracticalNotes.md)

After the code in Listing 2.3 is run , a new VkBuffer handle is created and placed in the `buffer` variable , the buffer is not yet fully usable because it first needs to be backed with memory. This operation is covered in "Device Memory Management" Later in this chapter 

## Formats and Support

while buffers are simple resources and do not have any notion of the format of the data they contain , images and buffer views (which will be covered later) do include information about their content. part of that information describes the format of the data in the resource . Some formats have special requirements / restrictions on their use in certain parts of the pipeline. For example, some formats might be readable but not writable , which common with compressed formats.

to get level of support for a format you can call :
`vkGetPhysicalDeviceFormatProperties()` , the prototype of which is:
```cpp
void vkGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice    physicalDevice,
    VkFormat            format,
    VkFormatProperties* pFormatProperties
);
```

*`physicalDevice`* = the physical device that you want to see support of the image in.

*`format`* = the format which you want to see level of support for.

*`pFormatProperties`* = properties of format will be output in instance of `VkFormatProperties` in this object.

VkFormatProperties prototype:
```cpp
typedef struct VkFormatProperties{
    VkFormatFeatureFlags    linearTilingFeatures;
    VkFormatFeatureFlags    optimalTilingFeatures;
    VKFormatFeatureFlags    bufferFeatures;
}VkFormatProperties;
```

an image can be in one of two primary tiling modes : linear in which data is laid out linearly in memory , and optimal in which data is laid out in highly optimized pattern which make efficient use of device's memory subsystem. 

*`linearTilingFeatures`* = level of support for a format in linear tiling mode

*`optimalTilingFeatures`* = level of support for a format in optimal tiling mode

*`bufferFeatures`* = level of support for a format when used in a buffer

the various bits that might be included in these bitfields are defined as below:

- `VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT` : The format may be used in read only images that will be sampled by shaders.

- `VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT` : Filter modes that include linear filtering may be used when this format is used for a sampled image.

- `VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT` : The format may be used in read-write images that can be read or written by shaders.

- `VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT` : The format may be used in read-write images that also support atomic operations by shaders.

- `VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT` : The format may be used in read-only texel buffers that will be read from shaders.

- `VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT` : The format may be used in read-write texel buffers that will be read from or written to by shaders.

- `VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT` : The format may be used in read-write texel buffers that support atomic operations by shaders.

- `VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT` : The format may be used as the source of the vertex data by the vertex-assembly stage of the graphics pipeline.

- `VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT` : The format may be used as the color attachment by the color-blend stage of the graphics pipeline

- `VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT` : Images with this format may be used as color attachments when blending is enabled.

- `VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMMENT_BIT` : The format may be used as depth, stencil or depth-stencil attachment.

- `VK_FORMAT_FEATURE_BLIT_SRC_BIT` : The format may be used as the source of data in an image copy operation.

- `VK_FORMAT_FEATURE_BLIT_DST_BIT` : The format may be used as the destination in an image copy operation.

Many formats will have a number of format support bits turned on. in fact , many formats are compulsory to support. A complete list of the mandatory formats is contained in the Vulkan spec. 

The `vkGetPhysicalDeviceFormatProperties()` only shows whether a format may be used at all under particular scenarios. for images especially , there may be more complex interaction between a specific format and its effect on the level of support within an image. So to get even more information about a format when used in images:

call : `vkGetPhysicalDeviceImageFormatProperties()` , the prototype of which is:

```cpp
VkResult vkGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice            physicalDevice,
    VkFormat                    format,
    VkImageType                 type,
    VKImageTiling               tiling,
    VkImageUsageFlags           usage,
    VkImageCreateFlags          flags,
    VkImageFormatProperties*    pImageFormatProperties
);
```

like `vkGetPhysicalDeviceFormatProperties()`, `vkGetPhysicalDeviceImageFormatProperties()` takes a `physicalDevice` and `format`

*`physicalDevice`* = physical device

*`format`* = format you are querying support for

*`type`* = The type of the image , 1D,2D or 3D :
-  `VK_IMAGE_TYPE_1D`
-  `VK_IMAGE_TYPE_2D`
-  `VK_IMAGE_TYPE_3D`

*`tiling`* = The tiling mode for the image:
- `VK_IMAGE_TILING_LINEAR` OR
- `VK_IMAGE_TILING_OPTIMAL`

*`usage`* = bitfield indicating how the image is to be used , discussed ,later in this chapter.

*`flags`* = should be set to the value used when creating the image which will use this format.

*`pImageFormatProperties`* = information about the level of support is written to this instance of `VkImageFormatProperties`.

The prototype of `VkImageFormatProperties` :

```cpp
typedef struct VkImageFormatProperties{
    VkExtent3D          maxExtent;
    uint32_t            maxMipLevels;
    uint32_t            maxArrayLayers;
    VkSampleCountFlags  sampleCounts;
    VkDeviceSize        maxResourceSize;
}VkImageFormatProperties;
```

*`maxExtent`* = maximum size of an image that can be created with this format. For example, images with fewer bits per pixel may support creating larger images than those with wider bits per pixel
prototype of `VkExtent3D`

```cpp
typedef struct VkExtent3D{
    uint32_t width;
    uint32_t height;
    uint32_t depth;
}VkExtent3D;
```

*`maxMipLevels`* = maximum number of mipmap levels supported for an image of the requested format with the other parameters passed to `vkGetPhysicalDeviceImageFormatProperties()` , maxMipLevels will report 
`log2(max(extent.x,extent.y,extent.z))` for the image when mipmaps are supported , or 1 when mipmaps are not supported.

*`maxArrayLayers`* = maximum number of array layers supported for the image. this is supposed to be fairy high number if arrays are supported and 1 if arrays are not supported ,

*`sampleCounts`* = supported sample counts , bitfield containing one bit for each supported sample count. if bit n is set then images with 2^n samples are supported. for example if bit 0 is set then it supports 1 sample per pixel if bit 2 is set then 2 for bit 3 it supports 4 samples and so on , if the format is supported at all atleast one bit will be set

maxResourceSize = this field specifies maximum size in bytes that a resource in this format might be. this should not be confused with maximum extent, which reports maximum size in dimension that an image supports. if an implementation reports 16384 x 16384 pixels x 2048 Layers , with a format containing 128 bits per pixel then creating an image of the maximum extent in every dimension would produce 8 TiB of Image data.

