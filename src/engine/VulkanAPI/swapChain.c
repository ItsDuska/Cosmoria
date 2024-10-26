#include "swapChain.h"
//#include "window/windowDefines.h"
#include "vulkanDevice.h"
#include "util/logger/logger.h"
#include "util/math/math.h"
//#include "window/window.h"
#include "engine/window/window.h"

typedef struct LocalSwapChainContext
{
    u32 imageViewCount;
    VkImageView* views;
    u32 imageCount;
    VkImage* images;
    u32 frameBufferCount;
    VkFramebuffer* frameBuffers;

    u32 syncObjectCount;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;

    VulkSwapChainContext* core;

}  LocalSwapChainContext;

static LocalSwapChainContext* localContext;


static VkImageView createImageView(VkImage image, VkFormat format,VulkanContext* context)
{
    VkImageViewCreateInfo viewInfo = {0};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(context->device, &viewInfo, NULL, &imageView) != VK_SUCCESS)
    {
        cosmoriaLogMessage(COSMORIA_LOG_ERROR, "Failed to create texture image view!");
        imageView = VK_NULL_HANDLE;
    }
    return imageView;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, u32 formatSize)
{
    for (u32 i = 0; i < formatSize; i++)
    {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormats[i];
		}
    }
    // not sure if we ever get here but just in case. This might break things tho. :p
    return availableFormats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, u32 modeCount)
{
    for (u32 i = 0; i < modeCount; i++)
    {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentModes[i];
		}
    }
}

static VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR* capabilities)
{
    if (capabilities->currentExtent.width != UINT32_MAX)
    {
        return capabilities->currentExtent;
    }
    
    Vec2u windowSize = getWindowSize();
    VkExtent2D windowExtent = {windowSize.x,windowSize.y};

    windowExtent.width = CLAMP(windowExtent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
    windowExtent.height = CLAMP(windowExtent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

    return windowExtent;
}

static b8 createVulkSwapChain(VulkanContext* context)
{
    localContext = (LocalSwapChainContext*) malloc(sizeof(LocalSwapChainContext));
    localContext->core = (VulkSwapChainContext*) malloc(sizeof(VulkSwapChainContext));

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(context->physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats,swapChainSupport.formatCount);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes,swapChainSupport.presentModeCount);

    localContext->core->extent = chooseSwapExtent(&swapChainSupport.capabilities);
    localContext->core->format = surfaceFormat.format;

    u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

    VkSwapchainCreateInfoKHR createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = context->surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = localContext->core->extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(context->physicalDevice);
    u32 queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
	}

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(context->device, &createInfo, NULL, &localContext->core->swapChain) != VK_SUCCESS)
	{
		cosmoriaLogMessage(COSMORIA_LOG_ERROR,"Failed to create swapchain!");
        return COSMORIA_FAILURE;
	}
    
    vkGetSwapchainImagesKHR(context->device, localContext->core->swapChain, &imageCount, NULL);
    localContext->images = (VkImage*)malloc(sizeof(VkImage)*imageCount);
    localContext->imageCount = imageCount;
    vkGetSwapchainImagesKHR(context->device, localContext->core->swapChain, &imageCount, localContext->images);

    return COSMORIA_SUCCESS;
}

static b8 createImageViews(VulkanContext* context)
{
    localContext->views = malloc(sizeof(VkImageView)*localContext->imageCount);
    localContext->imageViewCount = localContext->imageCount;

    for (u32 i = 0; i < localContext->imageCount; i++)
    {
        localContext->views[i] = createImageView(localContext->images[i], localContext->core->format, context);
        if (localContext->views[i] == VK_NULL_HANDLE)
        {
            return COSMORIA_FAILURE;
        }
    }

    return COSMORIA_SUCCESS;
}

static b8 createRenderPass(VulkanContext* context)
{
    VkAttachmentDescription colorAttachment = {0};
	colorAttachment.format = localContext->core->format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {0};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[1] = { colorAttachment};
	VkSubpassDependency dependencies[1] = { dependency};

	VkRenderPassCreateInfo renderPassInfo = {0};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachments[0];
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependencies[0];

    if (vkCreateRenderPass(context->device,&renderPassInfo,NULL, &(localContext->core->renderPass)))
    {
        cosmoriaLogMessage(COSMORIA_LOG_ERROR, "Failed to create render pass!");
        return COSMORIA_FAILURE;
    }

    return COSMORIA_SUCCESS;
}

static b8 createFramebuffers(VulkanContext* context)
{
    localContext->frameBufferCount = localContext->imageViewCount;
    localContext->frameBuffers = malloc(sizeof(VkFramebuffer) * localContext->frameBufferCount);

    for (u32 i= 0; i < localContext->frameBufferCount; i++)
    {
        VkFramebufferCreateInfo framebufferInfo = {0};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = localContext->core->renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &(localContext->views[i]);
		framebufferInfo.width = localContext->core->extent.width;
		framebufferInfo.height = localContext->core->extent.height;
		framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(context->device, &framebufferInfo, NULL, &(localContext->frameBuffers[i])) != VK_SUCCESS)
		{
            cosmoriaLogMessage(COSMORIA_LOG_ERROR, "Failed to create framebuffer!");
            return COSMORIA_FAILURE;
        }
    }

    return COSMORIA_SUCCESS;
}


static b8 createSyncObjects(VulkanContext* context)
{
    localContext->imageAvailableSemaphores = malloc(sizeof(VkSemaphore)*context->maxFramesInFlight);
    localContext->renderFinishedSemaphores = malloc(sizeof(VkSemaphore)*context->maxFramesInFlight);
    localContext->inFlightFences = malloc(sizeof(VkFence)*context->maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo = {0};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {0};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u32 i = 0; i < context->maxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &localContext->imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &localContext->renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(context->device,     &fenceInfo,     NULL, &localContext->inFlightFences[i])           != VK_SUCCESS)
		{
			cosmoriaLogMessage(COSMORIA_LOG_ERROR, "Failed to create semaphores and fences for swapchain!");
            return COSMORIA_FAILURE;
		}
    }

    return COSMORIA_SUCCESS;
}

b8 createVulkSwapChainContext(WindowAPICore *window)
{
    VulkanContext* context = getVulkanContext();

    if (!createVulkSwapChain(context))
    {
        return COSMORIA_FAILURE;
    }
    
    if (!createImageViews(context))
    {
        return COSMORIA_FAILURE;
    }

    if (!createRenderPass(context))
    {
        return COSMORIA_FAILURE;
    }

    if (!createFramebuffers(context))
    {
        return COSMORIA_FAILURE;
    }

    if (!createSyncObjects(context))
    {
        return COSMORIA_FAILURE;
    }

    return COSMORIA_SUCCESS;
}


static void cleanupSwapChain(VulkanContext* context)
{
    if (localContext->frameBufferCount == NULL)
    {
        return;
    }
    for (u32 i = 0; i < localContext->frameBufferCount; i++)
    {
        vkDestroyFramebuffer(context->device,localContext->frameBuffers[i],NULL);
    }

    for (u32 i = 0; i < localContext->imageViewCount; i++)
    {
        vkDestroyImageView(context->device,localContext->views[i],NULL);
    }

    vkDestroySwapchainKHR(context->device, localContext->core->swapChain, NULL);
}

b8 recreateVulkSwapChain(void)
{
    VulkanContext* context = getVulkanContext();
    Vec2u winSize = getWindowSize();
    while (winSize.x == 0 || winSize.y == 0)
	{
		winSize = getWindowSize();
		processMessage();
	}

    vkDeviceWaitIdle(context->device);

    cleanupSwapChain(context);
	createVulkSwapChain(context);
	createImageViews(context);
	createFramebuffers(context);
}


void destroySwapChain(void)
{
    if (localContext == NULL)
    {
        return;
    }

    VulkanContext* context = getVulkanContext();
    cleanupSwapChain(context);
    vkDestroyRenderPass(context->device, localContext->core->renderPass, NULL);

    for (u32 i = 0; i < context->maxFramesInFlight; i++)
    {
        vkDestroySemaphore(context->device, localContext->imageAvailableSemaphores[i], NULL);
		vkDestroySemaphore(context->device, localContext->renderFinishedSemaphores[i], NULL);
		vkDestroyFence(context->device, localContext->inFlightFences[i], NULL);
    }

    free(localContext->core);

    free(localContext->frameBuffers);
    free(localContext->images);
    free(localContext->views);

    free(localContext);
}

