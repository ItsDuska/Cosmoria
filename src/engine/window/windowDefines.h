#ifndef COSMORIA_WINDOW_DEFINES
#define COSMORIA_WINDOW_DEFINES

#include "../../util/defines.h"
#include "Windows.h"

typedef struct WindowInfo
{
    const char* name;
    u32 width;
    u32 height;
} WindowInfo;


typedef struct WindowAPICore
{
    HINSTANCE hInstance;
    HWND hWnd;
} WindowAPICore;


typedef struct WindowHandle
{
    WindowAPICore handle;
    const wchar_t* clss_name;
    u32 width;
    u32 height;
} WindowHandle;

#endif