#include "engineCore.h"
#include "window/window.h"
#include "VulkanAPI/vulkanDevice.h"
#include "VulkanAPI/swapChain.h"

#include <stdio.h>

static b8 initFailed = false;

void createEngine(void)
{
    WindowInfo info;
    info.height = 800;
    info.width = 800;

    const char* originalName = "guh, buh? waaaaa...";
    info.name = (char*)malloc(strlen(originalName) + 1);
    strcpy(info.name, originalName);

    if (!createWindow(&info))
    {
        initFailed = true;
        free(info.name);
        return; 
    }

    free(info.name);

    WindowAPICore* windowPtr = getWindowHandlePtr();
    if (!createVulkanContext(windowPtr))
    {
        initFailed = true;
    }

    if (!createVulkSwapChainContext(windowPtr))
    {
        initFailed = true;
    }

}

void runEngine(void)
{
    if (initFailed)
    {
        printf("guuuuuh\n");
        return;
    }
    
    while (processMessage())
    {

    }
    
}

void destroyEngine(void)
{
    destroySwapChain();
    destroyVulkanContext();

    destroyWindow();

    printf("exiting program\n");
}
