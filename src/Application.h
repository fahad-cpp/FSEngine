#ifndef APPLICATION
#define APPLICATION
#include <vulkan/vulkan.h>
#include <vector>
#include "Allocator.h"
class Application {
    VkInstance                      m_instance;
    VkPhysicalDevice                m_physicalDevice;
    VkDevice                        m_device;

public:

    VkResult    init();
    void        run();
    void        cleanup();
};
#endif