#ifndef COSMORIA_VULKAN_DEVICE
#define COSMORIA_VULKAN_DEVICE

#include "util/defines.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>


struct WindowAPICore;

typedef struct VulkanContext
{
    VkSurfaceKHR surface;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkCommandPool commandPool;
} VulkanContext;


b8 createVulkanContext(struct WindowAPICore* window);

void destroyVulkanContext(void);

#endif