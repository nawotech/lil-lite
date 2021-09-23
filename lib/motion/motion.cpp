#include "motion.h"
#define ACCEL_SAMPLE_RATE 3.125 // HZ - Samples per second - 0.781, 1.563, 3.125, 6.25, 12.5, 25, 50, 100, 200, 400, 800, 1600Hz
#define ACCEL_RANGE 2           // Accelerometer range = 2, 4, 8, 16g

Motion::Motion(KXTJ3 *accel)
{
    _Accel = accel;
    _current_state = MOTION_STOPPED;
}

void Motion::begin()
{
    _Accel->begin(ACCEL_SAMPLE_RATE, ACCEL_RANGE);
    _Accel->intConf(20, 1, 20, true); // set accelerometer to cause interrupt on motion
    _Tmr.reset();
}

motion_state_t Motion::get_state()
{
    return _current_state;
}

void Motion::update()
{
    switch (_current_state)
    {
    case MOTION_STOPPED:
        if (_Accel->isMotionInt())
        {
            // movement detected, this means movement started
            _current_state = MOTION_MOVING;
        }
        break;

    case MOTION_START_MOVING:
        break; // todo add start moving state

    case MOTION_MOVING:
        if (!_Accel->isMotionInt())
        {
            // no activity counter expired, this means movement stopped
            _current_state = MOTION_STOPPED;
        }
        break;

    case MOTION_BRAKING:
        break; // todo add braking state

    case MOTION_PARKED:
        break; // todo add state
    }
}