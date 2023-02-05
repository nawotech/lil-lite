#ifndef GROWRAND_H
#define GROWRAND_H

#include "patman.h"

class GrowRand : public Pattern
{
public:
    GrowRand(Patman *Pat, RgbColor color);
    void start();

private:
    void update_function(const AnimationParam &param);
    void update_function_2(const AnimationParam &param);
    RgbColor _color;
    Patman *_Pat;
};

#endif