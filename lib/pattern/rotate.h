#ifndef ROTATE_H
#define ROTATE_H

#include "patman.h"

class Rotate : public Pattern
{
public:
    Rotate(Patman *Pat, RgbColor color);
    void start();

private:
    void update_function(const AnimationParam &param);
    void update_function_2(const AnimationParam &param);
    RgbColor _color;
    void re_start();
    Patman *_Pat;
};

#endif