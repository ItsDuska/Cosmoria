#ifndef COSMORIA_WINDOW
#define COSMORIA_WINDOW

#include "../../config/defines.h"
#include "Windows.h"

typedef struct WindowInfo
{
    const char* name;
    u32 width;
    u32 height;
} WindowInfo;


typedef struct WindowHandle
{
    HINSTANCE hInstance;
    HWND hWnd;
    const wchar_t* clss_name;
    u32 width;
    u32 height;
} WindowHandle;


void createWindow(WindowInfo *info);

u8 processMessage(void);

void destroyWindow(void);

#endif