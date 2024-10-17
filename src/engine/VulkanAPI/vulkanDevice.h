#ifndef COSMORIA_VULKAN_DEVICE
#define COSMORIA_VULKAN_DEVICE

#include "util/defines.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>


struct WindowAPICore;


typedef struct QueueFamilyIndices
{
	u32 graphicsFamily;
	u32 presentFamily;
} QueueFamilyIndices;


typedef struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR* formats;
	u32 formatCount;
	VkPresentModeKHR* presentModes;
	u32 presentModeCount;
} SwapChainSupportDetails;

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


b8 queueFamilyIsComplete(QueueFamilyIndices family);


SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

void destroySwapChainSupportDetails(SwapChainSupportDetails* data);
//SwapChainSupportDetails getSwapChainSupport();
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

#endif