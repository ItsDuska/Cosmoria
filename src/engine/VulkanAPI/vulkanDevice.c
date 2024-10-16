#include "vulkanDevice.h"
#include <stdio.h>

// TODO: TEE JOTAIN TÄLLE. Includaa tää kun on debug build vaan.
#define ENABLE_VALIDATION_LAYER

#ifdef ENABLE_VALIDATION_LAYER
const b8 VALIDATION_LAYERS = true;
#else
const b8 VALIDATION_LAYERS = false;
#endif


const char* validationLayer = "VK_LAYER_KHRONOS_validation";

typedef struct LocalVkContext
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT dbgMessenger;
} LocalVkContext;


static LocalVkContext* localVulkCotext;


static VKAPI_ATTR VkBool32 VKAPI_CALL vulkDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 6);
	printf("VALIDATION LAYER: ");
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	printf("%s\n",pCallbackData->pMessage);
	return VK_FALSE;
}

static b8 supportVulkLayers(void)
{
    u32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	const VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(layerCount * sizeof(VkLayerProperties));
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
        printf("NO VALIDATION LAYERS FOUND?\n");
        return COSMORIA_FAILURE;
    }

	return COSMORIA_SUCCESS;
}

#ifdef ENABLE_VALIDATION_LAYER
static void getSupportedExtensions(void)
{
    u32 extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

	const VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(extensionCount * sizeof(VkExtensionProperties));
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

	printf("Available extensions:\n");

	for (u32 i = 0; i < extensionCount; i++)
	{
        printf("\t-%s\n", extensions[i].extensionName);
	}
    printf("\n");

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

b8 createVulkInstance(void)
{
    if (VALIDATION_LAYERS && !supportVulkLayers())
    {
        return COSMORIA_FAILURE;
    }

    localVulkCotext = (LocalVkContext*)malloc(sizeof(LocalVkContext));
    memset(localVulkCotext, 0, sizeof(LocalVkContext));

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

    if (vkCreateInstance(&createInfo, NULL, &localVulkCotext->instance) != VK_SUCCESS)
	{
        printf("ERROR: Failed to create Instance!\n");
        return COSMORIA_FAILURE;
    }

#ifdef ENABLE_VALIDATION_LAYER
    if (createDebugUtilsMessengerEXT(localVulkCotext->instance,
        &debugCreateInfo,
        NULL,
        &localVulkCotext->dbgMessenger)
        != VK_SUCCESS)
	{
        printf("ERROR: Failed to set up debug messenger!\n");
        return COSMORIA_FAILURE;
	}
#endif

    return COSMORIA_SUCCESS;
}

void destroyVulkanInstance(void)
{
#ifdef ENABLE_VALIDATION_LAYER
    destroyDebugUtilsMessengerEXT(localVulkCotext->instance, localVulkCotext->dbgMessenger, NULL);
#endif
    vkDestroyInstance(localVulkCotext->instance, NULL);

    free(localVulkCotext);
}
