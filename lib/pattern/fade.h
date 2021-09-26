#ifndef FADE_H
#define FADE_H

#include "patman.h"

class Fade : public Pattern
{
public:
    Fade(Patman *Pat, RgbColor color);
    void start();

private:
    void update_function(const AnimationParam &param);
    RgbColor _color;
    uint16_t _speed;
    Patman *_Pat;
};

#endif