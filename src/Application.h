#ifndef APPLICATION
#define APPLICATION
#include "VulkanCore.h"
class Application {
    VulkanCore vkCore = {};

  public:
    Application() {
        init();
    }
    ~Application() {
        cleanup();
    }
    void init();
    void run();
    void cleanup();
    void handleInput();
};
#endif
