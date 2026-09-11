#ifndef LOGGING_H
#define LOGGING_H
#include <iostream> // IWYU pragma: keep
#define LOG_ERROR(str)\
std::cerr << "[FSENGINE ERROR]: " << str << " : " << __FUNCTION__ << "() : line " << __LINE__ << "\n";
#define LOG_INFO(str)\
std::cout << "[FSENGINE]: " << str << "\n";
#endif