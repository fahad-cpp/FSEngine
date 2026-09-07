#ifndef TIMER_H
#define TIMER_H
#include <chrono>
struct Timer{
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    float diff=0.f;
};
void startTimer(Timer& timer);
void endTimer(Timer& timer);
#endif