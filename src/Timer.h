#ifndef TIMER_H
#define TIMER_H
#include <chrono>
#include <iostream> // IWYU pragma: keep
#include "Logging.h"
struct Timer{
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    float diff=0.f;
};
#define TIME_FUNC(label,timer,func)\
    startTimer(timer);\
    func;\
    endTimer(timer);\
    LOG_INFO(label << " : " << (timer.diff / 1000.f) << " ms");
void startTimer(Timer& timer);
void endTimer(Timer& timer);
float microsecToFPS(float microSeconds);
#endif