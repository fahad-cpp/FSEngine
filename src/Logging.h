#ifndef LOGGING_H
#define LOGGING_H
#include <iostream> // IWYU pragma: keep
#define LOG_ERROR(str)\
std::cerr << "\r[FSENGINE ERROR]: " << str << " : " << __FUNCTION__ << "() : line " << __LINE__ << " in file " << __FILE__ << "\n";
#define LOG_INFO(str)\
std::cout << "\r[FSENGINE]: " << str << "\n";
#define LOG_LIVE(str)\
std::cout << "\r[FSENGINE]: " << str << std::flush;
#endif