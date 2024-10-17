#include "window.h"
#include <stdio.h>
#include "util/logger/logger.h"


static WindowHandle* localHandle;

LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
	{
        case WM_CLOSE:
            DestroyWindow(localHandle->handle.hWnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

b8 createWindow(WindowInfo *info)
{
    localHandle = malloc(sizeof(WindowHandle));
    localHandle->width = info->width;
    localHandle->height = info->height;

    wchar_t *CLASS_NAME = L"COSMORIA_CLASS";
    localHandle->clss_name = malloc((lstrlenW(CLASS_NAME) + 1) * sizeof(wchar_t));
    wcscpy(localHandle->clss_name, CLASS_NAME);

    localHandle->handle.hInstance = GetModuleHandle(NULL);

    WNDCLASS wndClass = {0};  
    wndClass.lpszClassName = localHandle->clss_name;
    wndClass.hInstance = localHandle->handle.hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);  
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);  
    wndClass.lpfnWndProc = windowProc;  
    wndClass.style = CS_HREDRAW | CS_VREDRAW;  
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wndClass))
    {
        DWORD error = GetLastError();
        cosmoriaLogMessage(COSMORIA_LOG_ERROR,"Failed to register window class. Error code: ");
        printf("%lu\n", error);
        return COSMORIA_FAILURE;
    }

    DWORD style = WS_THICKFRAME |
        WS_CAPTION | WS_MINIMIZEBOX |
        WS_SYSMENU | WS_MAXIMIZEBOX |
        WS_OVERLAPPED;

    RECT rect = {0};
    rect.left = 250;
    rect.top = 250;
    rect.right = rect.left + info->width;
    rect.bottom = rect.top + info->height;
    AdjustWindowRect(&rect, style, FALSE);

    int needed = MultiByteToWideChar(CP_UTF8, 0, info->name, -1, NULL, 0);
    if (needed == 0)
    {
        cosmoriaLogMessage(COSMORIA_LOG_ERROR,"Error converting window name from char to wide characters.\n");
        return COSMORIA_FAILURE;
    }
    
    wchar_t* nameWide = (wchar_t*)malloc(needed * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, info->name, needed, nameWide, needed);

    localHandle->handle.hWnd = CreateWindowEx(
        0,
        localHandle->clss_name,
        nameWide,
        style,
        rect.left, rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        localHandle->handle.hInstance,
        NULL
    );

    if (!localHandle->handle.hWnd)
    {
        DWORD error = GetLastError();

        cosmoriaLogMessage(COSMORIA_LOG_ERROR,"Failed to create window. Error code: ");
        printf("%lu\n", error);
        free(nameWide);
        return COSMORIA_FAILURE;
    }

    ShowWindow(localHandle->handle.hWnd, SW_SHOW);
    UpdateWindow(localHandle->handle.hWnd);

    free(nameWide);
    return COSMORIA_SUCCESS;
}

b8 processMessage(void)
{
    MSG msg;
		
    while (PeekMessage(&msg, NULL, 0u, 0u, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

void destroyWindow(void)
{
    UnregisterClass(localHandle->clss_name, localHandle->handle.hInstance);
    free(localHandle->clss_name);
    free(localHandle);
}

WindowAPICore *getWindowHandlePtr(void)
{
    return &localHandle->handle;
}
