#ifndef GROW_H
#define GROW_H

#include "patman.h"

class Grow : public Pattern
{
public:
    Grow(Patman *Pat, RgbColor color);
    void start();

private:
    void update_function(const AnimationParam &param);
    void update_function_2(const AnimationParam &param);
    Patman *_Pat;
};

#endif