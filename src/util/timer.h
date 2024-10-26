#ifndef COSMORIA_TIMER
#define COSMORIA_TIMER
#include <windows.h>

typedef struct CTimer
{
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    LARGE_INTEGER frequency;
} CTimer;

// Starts the timer.
void startTimer(CTimer* timer)
{
    QueryPerformanceFrequency(&(timer->frequency));
    QueryPerformanceCounter(&(timer->startTime));
}

// stops the clock and returns the dt.
double getDeltaTime(CTimer* timer)
{
    QueryPerformanceCounter(&(timer->endTime));

    double startSeconds = (double)timer->startTime.QuadPart / (double)timer->frequency.QuadPart;
    double endSeconds = (double)timer->endTime.QuadPart / (double)timer->frequency.QuadPart;

    return endSeconds - startSeconds;
}

// guh. 
void sleepForFPS(double deltaTime, double targetFPS)
{
    double targetFrameTime = 1.0 / targetFPS;
    double sleepTime = targetFrameTime - deltaTime;
    if (sleepTime > 0)
    {
        Sleep((DWORD)(sleepTime * 1000));  // milliseconds -> sleep
    }
}
#endif