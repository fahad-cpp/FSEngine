#include "Window.h"
#ifdef _WIN32
LRESULT windowProcedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProcA(window, msg, wParam, lParam);
}
HWND createWin32Window() {
    WNDCLASSA mWindowClass = {};
    mWindowClass.lpfnWndProc = windowProcedure;
    mWindowClass.lpszClassName = "ClassName";
    mWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    mWindowClass.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClassA(&mWindowClass);

    HWND window = CreateWindowA(mWindowClass.lpszClassName, "VulkanApp Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 720, 720, NULL, NULL, NULL, NULL);

    return window;
}
#elif __linux__
Window createXlibWindow(){
    Display* display = XOpenDisplay(0);
    Window RootWindow = XDefaultRootWindow(display);
    Window window = XCreateSimpleWindow(display,RootWindow,0,0,720,720,0,0,0x000000);
    XMapWindow(display,window);
    XFlush(display);

    return window;
}
void deleteXlibWindow(Window& window){
    Display* display = XOpenDisplay(0);
    XDestroyWindow(display,window);
}
#endif
