#ifndef MOTION_H
#define MOTION_H

#include "kxtj3-1057.h"
#include "timer.h"

typedef enum
{
    MOTION_STOPPED,
    MOTION_START_MOVING,
    MOTION_MOVING,
    MOTION_BRAKING,
    MOTION_PARKED
} motion_state_t;

class Motion
{
public:
    Motion(KXTJ3 *accel);
    void begin();
    motion_state_t get_state();
    void update();

private:
    KXTJ3 *_Accel;
    motion_state_t _current_state;
    Timer _Tmr;
};

#endif