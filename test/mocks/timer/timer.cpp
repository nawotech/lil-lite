#include "timer.h"

bool Timer::time_passed(unsigned long time_ms)
{
    if (ms > time_ms)
    {
        Timer::reset();
        return true;
    }
    return false;
}

uint32_t Timer::get_ms()
{
    return ms;
}

void Timer::MOCK_time_passed(uint32_t ms_passed)
{
    ms = ms_passed;
}

void Timer::reset()
{
    ms = 0;
}