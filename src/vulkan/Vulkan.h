#ifndef VULKAN_INCLUDE_FS
#define VULKAN_INCLUDE_FS
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>
#endif