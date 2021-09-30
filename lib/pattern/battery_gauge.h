#ifndef BATTERY_GAUGE_H
#define BATTERY_GAUGE_H

#include "patman.h"
#include "power.h"

class BatteryGauge : public Pattern
{
public:
    BatteryGauge(Patman *Pat, Power *Pwr);
    void start();

private:
    void update_function(const AnimationParam &param);
    void update_function_2(const AnimationParam &param);
    RgbColor _color;
    void re_start();
    uint16_t _bat_leds;
    Patman *_Pat;
    Power *_Pwr;
    bool _reverse = false;
};

#endif