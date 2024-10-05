#include "createEngine.h"
#include "window/window.h"
#include <stdio.h>

void createEngine(void)
{
    WindowInfo info;
    info.height = 800;
    info.width = 800;

    const char* originalName = "guh, buh? waaaaa...";
    info.name = (char*)malloc(strlen(originalName) + 1);
    strcpy(info.name, originalName);

    createWindow(&info);

    free(info.name);
}

void runEngine(void)
{
    while (processMessage())
    {

    }

    destroyWindow();
    printf("exiting program\n");
}

void destroyEngine(void)
{

}
