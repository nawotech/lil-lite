#ifndef POWER_OFF_H
#define POWER_OFF_H

#include "patman.h"

class PowerOff : public Pattern
{
public:
    PowerOff(Patman *Pat);
    void start();

private:
    void update_function(const AnimationParam &param);
    RgbColor _color;
    Patman *_Pat;
};

#endif