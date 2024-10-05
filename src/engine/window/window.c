#include "window.h"
#include <stdio.h>


static WindowHandle* handle;

LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
	{
        case WM_CLOSE:
            DestroyWindow(handle->hWnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void createWindow(WindowInfo *info)
{
    handle = malloc(sizeof(WindowHandle));
    handle->width = info->width;
    handle->height = info->height;

    wchar_t *CLASS_NAME = L"COSMORIA_CLASS";
    handle->clss_name = malloc((lstrlenW(CLASS_NAME) + 1) * sizeof(wchar_t));
    wcscpy(handle->clss_name, CLASS_NAME);

    handle->hInstance = GetModuleHandle(NULL);

    WNDCLASS wndClass = {0};  
    wndClass.lpszClassName = handle->clss_name;
    wndClass.hInstance = handle->hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);  
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);  
    wndClass.lpfnWndProc = windowProc;  
    wndClass.style = CS_HREDRAW | CS_VREDRAW;  
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wndClass))
    {
        DWORD error = GetLastError();
        printf("Failed to register window class. Error code: %lu\n", error);
        return;
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
        printf("Error converting window name from char to wide characters.\n");
        return;
    }
    
    wchar_t* nameWide = (wchar_t*)malloc(needed * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, info->name, needed, nameWide, needed);

    handle->hWnd = CreateWindowEx(
        0,
        handle->clss_name,
        nameWide,
        style,
        rect.left, rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        handle->hInstance,
        NULL
    );

    if (!handle->hWnd)
    {
        DWORD error = GetLastError();
        printf("Failed to create window. Error code: %lu\n", error);
        free(nameWide);
        return;
    }

    ShowWindow(handle->hWnd, SW_SHOW);
    UpdateWindow(handle->hWnd);

    free(nameWide);
}

u8 processMessage(void)
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
    UnregisterClass(handle->clss_name, handle->hInstance);
    free(handle->clss_name);
    free(handle);
}
