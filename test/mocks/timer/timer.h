#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

class Timer
{

public:
    // returns true if time_ms has passed since reset was last called and resets timer, else false
    bool time_passed(unsigned long time_ms);

    uint32_t get_ms();

    // reset the timer
    void reset();

    void MOCK_time_passed(uint32_t ms_passed);

    uint32_t ms = 0;
};

#endif