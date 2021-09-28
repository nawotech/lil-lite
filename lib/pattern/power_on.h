#ifndef POWER_ON_H
#define POWER_ON_H

#include "patman.h"

class PowerOn : public Pattern
{
public:
    PowerOn(Patman *Pat);
    void start();

private:
    void update_function(const AnimationParam &param);
    RgbColor _color;
    Patman *_Pat;
};

#endif