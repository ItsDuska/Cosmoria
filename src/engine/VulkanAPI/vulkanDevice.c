#include "vulkanDevice.h"
#include <stdio.h>

#include "../window/windowDefines.h"
#include "util/logger/logger.h"

// TODO: TEE JOTAIN TÄLLE. Includaa tää kun on debug build vaan.
#define ENABLE_VALIDATION_LAYER

#ifdef ENABLE_VALIDATION_LAYER
const b8 VALIDATION_LAYERS = true;
#else
const b8 VALIDATION_LAYERS = false;
#endif



//TODO: Tee deque joka poistaa vulkan objetkeja järjestyksessä.


const char* validationLayer = "VK_LAYER_KHRONOS_validation";


typedef struct LocalVkContext
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT dbgMessenger;
    VulkanContext context;
} LocalVkContext;


static LocalVkContext* localVulkContext;


b8 queueFamilyIsComplete(QueueFamilyIndices family)
{
    return family.graphicsFamily != UINT32_MAX && family.presentFamily != UINT32_MAX;
}

VulkanContext *getVulkanContext(void)
{
    return &localVulkContext->context;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, localVulkContext->context.surface, &details.capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, localVulkContext->context.surface, &details.formatCount, NULL);

    if (details.formatCount != 0)
	{
        details.formats = (VkSurfaceFormatKHR*)malloc(details.formatCount * sizeof(VkSurfaceFormatKHR));
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, localVulkContext->context.surface, &details.formatCount, details.formats);
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, localVulkContext->context.surface, &details.presentModeCount, NULL);

    if (details.presentModeCount != 0)
	{
		details.presentModes = (VkPresentModeKHR*)malloc(details.presentModeCount * sizeof(VkPresentModeKHR));
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, localVulkContext->context.surface, &details.presentModeCount, details.presentModes);
	}

    return details;
}

void destroySwapChainSupportDetails(SwapChainSupportDetails *data)
{
    free(data->formats);
    free(data->presentModes);
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    indices.graphicsFamily = UINT32_MAX;
    indices.presentFamily = UINT32_MAX;

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

	VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	for (u32 i = 0; i < queueFamilyCount; i++)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, localVulkContext->context.surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		if (queueFamilyIsComplete(indices))
		{
			break;
		}
	}

    free(queueFamilies);
	return indices;
}

static b8 checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    u32 extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties* availableExtensions = (VkExtensionProperties*)malloc(extensionCount* sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    const char* deviceExtensions[3] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
	    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
    };

    const u16 neededExtensionCount = 3;
    b8 found;

    for (size_t i = 0; i < neededExtensionCount; i++)
    {
        found = false;
        
        for (size_t j = 0; j < extensionCount; j++)
        {
            if (strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            break;
            found = false;
        }
    }

    free(availableExtensions);
    return found;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	printf("VALIDATION LAYER: ");
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	printf("%s\n",pCallbackData->pMessage);
	return VK_FALSE;
}

static b8 supportVulkLayers(void)
{
    u32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(layerCount * sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    b8 layerFound = false;

    for (u32 i = 0; i< layerCount; i++)
    {
        //printf("-%s\n",availableLayers[i].layerName);
        if (strcmp(validationLayer, availableLayers[i].layerName) == 0)
        {
            layerFound = true;
            break;
        }
    }
    free(availableLayers);

    if (!layerFound)
    {
        cosmoriaLogMessage(COSMORIA_LOG_ERROR,"NO VALIDATION LAYERS FOUND?\n");
        return COSMORIA_FAILURE;
    }

	return COSMORIA_SUCCESS;
}

#ifdef ENABLE_VALIDATION_LAYER
static void getSupportedExtensions(void)
{
    u32 extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

	VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(extensionCount * sizeof(VkExtensionProperties));
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    cosmoriaLogMessage(COSMORIA_LOG_INFO,"Available extensions:");

	for (u32 i = 0; i < extensionCount; i++)
	{
        printf("\t-%s\n", extensions[i].extensionName);
	}

    free(extensions);
}

static VkResult createDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != NULL)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


static void destroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != NULL)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

#endif


static b8 suitableVulkPhysicalDevice(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
    printf("\t-%s\n",deviceProperties.deviceName);

    QueueFamilyIndices indices = findQueueFamilies(device);

    VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    b8 extensionsSupported = checkDeviceExtensionSupport(device);

    b8 swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = (!swapChainSupport.formatCount == 0 && !swapChainSupport.presentModeCount == 0);
        destroySwapChainSupportDetails(&swapChainSupport);
	}

    b8 isComplete = queueFamilyIsComplete(indices);

    return (isComplete &&
        extensionsSupported && 
        swapChainAdequate && 
        supportedFeatures.samplerAnisotropy);
}

static b8 pickVulkPhysicalDevice(void)
{
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(localVulkContext->instance, &deviceCount, NULL);

    if (deviceCount == 0)
    {
        cosmoriaLogMessage(COSMORIA_LOG_ERROR, "Failed to find GPUs with Vulkan support!\n");
        return COSMORIA_FAILURE;
    }

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(localVulkContext->instance, &deviceCount, devices);

    cosmoriaLogMessage(COSMORIA_LOG_INFO, "Physical devices available:");

    for (u32 i = 0; i < deviceCount; i++)
    {
        if (suitableVulkPhysicalDevice(devices[i]))
        {
            localVulkContext->context.physicalDevice = devices[i];
            break;
        }
    }

    free(devices);

    if (localVulkContext->context.physicalDevice == VK_NULL_HANDLE)
    {
        cosmoriaLogMessage(COSMORIA_LOG_ERROR, "Failed to find a suitable GPU!");
        return COSMORIA_FAILURE;
    }

    return COSMORIA_SUCCESS;
}


static b8 createVulCmdPool(void)
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(localVulkContext->context.physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {0};

	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

	if (vkCreateCommandPool(localVulkContext->context.device, &poolInfo, NULL, &localVulkContext->context.commandPool) != VK_SUCCESS)
	{
        cosmoriaLogMessage(COSMORIA_FAILURE, "Failed to create command pool!\n");
        return COSMORIA_FAILURE;
	}
    return COSMORIA_SUCCESS;
}

static b8 createVulkInstance(void)
{
    if (VALIDATION_LAYERS && !supportVulkLayers())
    {
        return COSMORIA_FAILURE;
    }

#ifdef ENABLE_VALIDATION_LAYER
    getSupportedExtensions();
#endif
    
    VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Cosmoria";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Cosmoria Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.pNext = NULL;

	VkInstanceCreateInfo createInfo;
    memset(&createInfo,0,sizeof(VkInstanceCreateInfo));
    
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

    const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
#ifdef ENABLE_VALIDATION_LAYER
        ,VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    createInfo.enabledExtensionCount = NELEMS(extensions);
    createInfo.ppEnabledExtensionNames = extensions;

#ifdef ENABLE_VALIDATION_LAYER
    createInfo.enabledLayerCount = 1u; // We only use one. ADD MORE IF NEEDED
	const char* validationLayers[] = { validationLayer };
    createInfo.ppEnabledLayerNames = validationLayers;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    memset(&debugCreateInfo, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));

    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = vulkDebugCallback;
	debugCreateInfo.pUserData = NULL;

    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)& debugCreateInfo;
#else
    createInfo.enabledLayerCount = 0;
	createInfo.pNext = NULL;
#endif

    if (vkCreateInstance(&createInfo, NULL, &localVulkContext->instance) != VK_SUCCESS)
	{
        cosmoriaLogMessage(COSMORIA_LOG_ERROR,"Failed to create Instance!\n");
        return COSMORIA_FAILURE;
    }

#ifdef ENABLE_VALIDATION_LAYER
    if (createDebugUtilsMessengerEXT(localVulkContext->instance,
        &debugCreateInfo,
        NULL,
        &localVulkContext->dbgMessenger)
        != VK_SUCCESS)
	{
        cosmoriaLogMessage(COSMORIA_LOG_ERROR,"Failed to set up debug messenger!\n");
        return COSMORIA_FAILURE;
	}
#endif

    return COSMORIA_SUCCESS;
}

static b8 createVulkSurface(WindowAPICore* window)
{
    VkWin32SurfaceCreateInfoKHR info;
	info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.pNext = NULL;
	info.flags = 0;
	info.hinstance = window->hInstance;
	info.hwnd = window->hWnd;

	if (vkCreateWin32SurfaceKHR(localVulkContext->instance, &info, NULL, &localVulkContext->context.surface) != VK_SUCCESS)
	{
        printf("Failed to create Window Surface for Win32!\n");
        return COSMORIA_FAILURE;
	}

    return COSMORIA_SUCCESS;
}

static b8 createVulkDevice(void)
{
    if (!pickVulkPhysicalDevice())
    {
        return COSMORIA_FAILURE;
    }

    QueueFamilyIndices indices = findQueueFamilies(localVulkContext->context.physicalDevice);
    f32 queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {0};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
	deviceFeatures.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
	deviceFeatures.robustBufferAccess = VK_TRUE;

    const u32 neededExtensionCount = 3;
    const char* deviceExtensions[3] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
	    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
    };

    VkDeviceCreateInfo createInfo= {0};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = neededExtensionCount;
	createInfo.ppEnabledExtensionNames = deviceExtensions;

#ifdef ENABLE_VALIDATION_LAYER

    createInfo.enabledLayerCount = 1;
    const char* validationLayers[1] = {"VK_LAYER_KHRONOS_validation"};
	createInfo.ppEnabledLayerNames = validationLayers;
#else
    createInfo.enabledLayerCount = 0;
#endif
    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {0};
    descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

    createInfo.pNext = &descriptorIndexingFeatures;


    if (vkCreateDevice(localVulkContext->context.physicalDevice, &createInfo, NULL, &localVulkContext->context.device) != VK_SUCCESS)
	{
        cosmoriaLogMessage(COSMORIA_LOG_ERROR, "Failed to create logical device!\n");
        return COSMORIA_FAILURE;
    }

    vkGetDeviceQueue(localVulkContext->context.device, indices.graphicsFamily, 0, &localVulkContext->context.graphicsQueue);
	vkGetDeviceQueue(localVulkContext->context.device, indices.presentFamily, 0, &localVulkContext->context.presentQueue);

    return COSMORIA_SUCCESS;
} 

// -----------------------
// MAIN STUFF FROM HEADER:
// -----------------------

b8 createVulkanContext(WindowAPICore *window)
{
    localVulkContext = (LocalVkContext*)malloc(sizeof(LocalVkContext));
    memset(localVulkContext, 0, sizeof(LocalVkContext));

    if (!createVulkInstance())
    {
        return COSMORIA_FAILURE;
    }

    if (!createVulkSurface(window))
    {
        return COSMORIA_FAILURE;
    }

    if (!createVulkDevice())
    {
        return COSMORIA_FAILURE;
    }

    if (!createVulCmdPool())
    {
        return COSMORIA_FAILURE;
    }
    
    // this value is hardcoded right now. Just change this if needed. :D
    localVulkContext->context.maxFramesInFlight = 2;

    return COSMORIA_SUCCESS;
}

void destroyVulkanContext(void)
{
    if (localVulkContext == NULL)
    {
        return;
    }

    vkDestroyCommandPool(localVulkContext->context.device,localVulkContext->context.commandPool, NULL);
    vkDestroyDevice(localVulkContext->context.device, NULL);
    vkDestroySurfaceKHR(localVulkContext->instance, localVulkContext->context.surface, NULL);

#ifdef ENABLE_VALIDATION_LAYER
    destroyDebugUtilsMessengerEXT(localVulkContext->instance, localVulkContext->dbgMessenger, NULL);
#endif
    vkDestroyInstance(localVulkContext->instance, NULL);

    free(localVulkContext);
}

