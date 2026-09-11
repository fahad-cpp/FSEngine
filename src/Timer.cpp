#include "Timer.h"
void startTimer(Timer& timer){
    timer.startTime = std::chrono::high_resolution_clock::now();
}
void endTimer(Timer& timer){
    timer.endTime = std::chrono::high_resolution_clock::now();
    timer.diff = std::chrono::duration<float,std::chrono::microseconds::period>(timer.endTime - timer.startTime).count();
}
float microsecToFPS(float microSeconds){
    return (1000000.f / microSeconds);;
}