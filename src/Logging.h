#ifndef LOGGING_H
#define LOGGING_H
#include <iostream> // IWYU pragma: keep
#define LOG_CREATION(result,objectName)\
if(result != VK_SUCCESS){\
    std::cerr << "[ERROR]:" << objectName << " creation" << "\n";\
}else{\
    std::cout << "[SUCCESS]:" << objectName << " creation" << "\n";\
}
#endif