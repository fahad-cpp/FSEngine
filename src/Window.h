#ifndef WINDOW_H
#define WINDOW_H
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
LRESULT windowProcedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
HWND createWin32Window();
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#endif