#ifndef VULKANSURFACE_H
#define VULKANSURFACE_H
#include <FSWindow.h>
#include "Instance.h"
#include "PhysicalDevice.h"
class VulkanSurface {
  private:
    VkSurfaceKHR m_surface;
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;

  public:
    VulkanSurface(VulkanInstance &instance, VulkanPhysicalDevice &physicalDevice, FS::Window &window);
    ~VulkanSurface();
    VkSurfaceCapabilitiesKHR getCapabilities();
    std::vector<VkSurfaceFormatKHR> getFormats();
    std::vector<VkPresentModeKHR> getPresentModes();
    inline VkSurfaceKHR get(){return m_surface;}
};
#endif