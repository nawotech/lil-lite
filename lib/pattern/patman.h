#ifndef PATMAN_H
#define PATMAN_H

#include "NeoPixelBus.h"
#include "NeoPixelAnimator.h"
#include "power.h"

class Pattern
{
public:
    virtual void start() = 0;
    void set_color(RgbColor color);
    float current_mA = 0.0;
    RgbColor _color;

private:
    virtual void update_function(const AnimationParam &param) = 0;
};

class Patman
{
public:
    Patman(uint16_t countPixels, uint8_t pin, Power *Pwr);
    void begin();
    void update();
    void set_pattern(Pattern *pattern);
    void blank_leds();
    bool is_running();

    NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixels;
    NeoPixelAnimator Anim;

private:
    Pattern *_current_pattern = 0;
    Power *_Pwr;
};

#endif