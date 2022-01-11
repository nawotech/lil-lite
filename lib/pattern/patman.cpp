#include "patman.h"

void Pattern::set_color(RgbColor color)
{
    _color = color;
}

Patman::Patman(uint16_t countPixels, uint8_t pin, Power *Pwr) : Pixels(countPixels, pin), Anim(1)
{
    _Pwr = Pwr;
}

void Patman::begin()
{
    Pixels.Begin();
    blank_leds();
}

void Patman::update()
{
    if (Anim.IsAnimating())
    {
        Anim.UpdateAnimations();
        Pixels.Show();
    }
}

void Patman::set_pattern(Pattern *pattern)
{
    if (_current_pattern != pattern)
    {
        _current_pattern = pattern;
        _current_pattern->start();
        _Pwr->set_battery_load_current(pattern->current_mA);
    }
}

void Patman::blank_leds()
{
    Anim.StopAll();
    Pixels.ClearTo(0);
    Pixels.Show();
    _Pwr->set_battery_load_current(0.0);
}

bool Patman::is_running()
{
    return Anim.IsAnimating();
}