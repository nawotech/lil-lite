#ifndef CHARGE_H
#define CHARGE_H

#include "patman.h"
#include "power.h"

class Charge : public Pattern
{
public:
    Charge(Patman *Pat, Power *Pwr);
    void start();

private:
    void update_function(const AnimationParam &param);
    RgbColor _color;
    RgbColor _prev_pixel_color;
    void re_start();
    uint16_t _led_num = 0;
    uint16_t _bat_leds = 2;
    Patman *_Pat;
    Power *_Pwr;
};

#endif