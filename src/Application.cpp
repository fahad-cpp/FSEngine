#include "Application.h"
#include "FSInput.h"
#include <iostream>
/*
    Last learned:
    -Copy Buffer Commands
*/
void Application::init() {
    std::cout << "\nApplication::init()\n\n";
}
void Application::handleInput() {
    FS::Input &input = vkCore.window->getInput();
    if (isDown(FS::Buttons::BUTTON_ESC)) {
        vkCore.window->close();
    }
}
void Application::run() {
    std::cout << "\nApplication::run()\n\n";
    while (vkCore.window->isOpen()) {
        handleInput();
        vkCore.window->processMessages();
        vkCore.drawFrame();
    }
}

void Application::cleanup() {
    std::cout << "\nApplication::cleanup()\n\n";
}
