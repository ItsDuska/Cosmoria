#ifndef COSMORIA_VULKAN_SWAPCHAIN
#define COSMORIA_VULKAN_SWAPCHAIN

#include <vulkan/vulkan.h>
#include "util/defines.h"

struct WindowAPICore;

typedef struct VulkSwapChainContext
{
    VkSwapchainKHR swapChain;
    VkFormat format;
    VkRenderPass renderPass;
    VkExtent2D extent;

} VulkSwapChainContext;


b8 createVulkSwapChainContext(struct WindowAPICore* window);
b8 recreateVulkSwapChain(void);
void destroySwapChain(void);

/*
VkRenderPass getRenderPass(void);
VkSemaphore getImageAvailableSemaphore(u32 index);
VkSemaphore getRenderFinishedSemaphore(u32 index);
VkFence getInFlightFence(u32 index);
VkSwapchainKHR getSwapchain(void);
VkFramebuffer getFrameBuffer(u32 index);
*/
#endif

/*
Tee jotain hassua tälle. Renderpasseja voidaan kai myöhemmin tarvita enemmän erilaisissa shader efekteissä sekä muissa. 
RenderTexture ja ne mm.

*/