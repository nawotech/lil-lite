#ifndef PATTERN_H
#define PATTERN_H

#include "NeoPixelBus.h"
#include "NeoPixelAnimator.h"

typedef enum
{
    PATTERN_TURN_ON,
    PATTERN_BATTERY_LEVEL,
    PATTERN_CHARGE,
    PATTERN_MOVE,
    PATTERN_STOPPED
} pattern_t;

class Pattern
{
public:
    Pattern(NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> *Leds);
    void begin();
    void update();
    void set_battery_gauge_percent(uint8_t percent);
    void start(pattern_t pattern);
    bool is_running();

private:
    NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> *_Pixels;
    NeoPixelAnimator _Anim;
    void pattern_move(const AnimationParam &param);
    void pattern_stop(const AnimationParam &param);
    void pattern_charge(const AnimationParam &param);
    bool _dir = false;
    RgbColor _color[3];
    uint16_t _color_n;
    uint16_t _num_leds;
    pattern_t _pattern = PATTERN_STOPPED;
    RgbColor _charge;
    uint8_t _battery_percent = 100;
};

#endif