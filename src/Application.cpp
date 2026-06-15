#include "Application.h"
#include <iostream>
/*
    Last learned:
    -Copy Buffer Commands
*/
void Application::init() {
    std::cout << "\nApplication::init()\n\n";
}

void Application::run() {
    std::cout << "\nApplication::run()\n\n";
    while(vkCore.window->isOpen()){
        vkCore.window->processMessages();
    }
}

void Application::cleanup() {
    std::cout << "\nApplication::cleanup()\n\n";
}
