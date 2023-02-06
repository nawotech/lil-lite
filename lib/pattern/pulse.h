#ifndef PULSE_H
#define PULSE_H

#include "patman.h"

class Pulse : public Pattern
{
public:
    Pulse(Patman *Pat, RgbColor color);
    void start();

private:
    void update_function(const AnimationParam &param);
    RgbColor _color;
    uint16_t _speed;
    Patman *_Pat;
};

#endif