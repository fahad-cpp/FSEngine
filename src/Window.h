#ifndef WINDOW_H
#define WINDOW_H
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
LRESULT windowProcedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
HWND createWin32Window();
#elif __linux__
#include <X11/Xlib.h>
Window createXlibWindow();
void deleteXlibWindow(Window& window);
#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#endif
#endif