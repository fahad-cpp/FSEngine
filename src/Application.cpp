#include "Application.h"
#include <iostream>
/*
    Last learned:
    -Command Buffers & Command Pools
*/
void Application::init() {
    std::cout << "\nApplication::init()\n\n";
}

void Application::run() {
    std::cout << "\nApplication::run()\n\n";
    std::cin.get();
}

void Application::cleanup() {
    std::cout << "\nApplication::cleanup()\n\n";
}
