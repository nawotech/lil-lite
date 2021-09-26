#ifndef PATMAN_H
#define PATMAN_H

#include "NeoPixelBus.h"
#include "NeoPixelAnimator.h"

class Pattern
{
public:
    virtual void start() = 0;

private:
    virtual void update_function(const AnimationParam &param) = 0;
};

class Patman
{
public:
    Patman(uint16_t countPixels, uint8_t pin);
    void begin();
    void update();
    void set_pattern(Pattern *pattern);

    NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixels;
    NeoPixelAnimator Anim;

private:
    Pattern *_current_pattern;
};

#endif