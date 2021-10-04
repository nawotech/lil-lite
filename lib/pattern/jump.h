#ifndef JUMP_H
#define JUMP_H

#include "patman.h"

class Jump : public Pattern
{
public:
    Jump(Patman *Pat, RgbColor color);
    void start();

private:
    void update_function(const AnimationParam &param);
    void update_function_2(const AnimationParam &param);
    RgbColor _color;
    Patman *_Pat;
};

#endif