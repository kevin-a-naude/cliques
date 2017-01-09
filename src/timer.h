
#pragma once

#ifndef TIMER_H
#define TIMER_H 1

#ifdef _WIN32

#include <windows.h>

typedef struct
{
    double seconds;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    LARGE_INTEGER frequency;
} timer_t;

static inline void timer_reset(timer_t* self)
{
    self->seconds = 0.0;
    self->startTime.QuadPart = 0;
    self->endTime.QuadPart = 0;
    QueryPerformanceFrequency(&self->frequency);
}

static inline void timer_start(timer_t* self)
{
    QueryPerformanceCounter(&self->startTime);
}

static inline void timer_stop(timer_t* self)
{
    double time;
    QueryPerformanceCounter(&self->endTime);
    time = ((double)(self->endTime.QuadPart - self->startTime.QuadPart) / self->frequency.QuadPart);
    if (time >= 0.0) self->seconds += time;
}

#else

#include <stddef.h>
#include <sys/time.h>

typedef struct
{
    double seconds;
    struct timeval startTime;
    struct timeval endTime;
} timer_t;

static inline void timer_reset(timer_t* self)
{
    self->seconds = 0.0;
    self->startTime.tv_sec = self->startTime.tv_usec = 0;
    self->endTime.tv_sec = self->endTime.tv_usec = 0;
}

static inline void timer_start(timer_t* self)
{
    gettimeofday(&self->startTime, NULL);
}

static inline void timer_stop(timer_t* self)
{
    double time;
    gettimeofday(&self->endTime, NULL);
    time = ((double)(self->endTime.tv_sec) + 0.000001*self->endTime.tv_usec) - ((double)(self->startTime.tv_sec) + 0.000001*self->startTime.tv_usec);
    if (time >= 0.0) self->seconds += time;
}

#endif

#endif
