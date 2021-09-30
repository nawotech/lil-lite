#include "patman.h"

Patman::Patman(uint16_t countPixels, uint8_t pin) : Pixels(countPixels, pin), Anim(1)
{
}

void Patman::begin()
{
    Pixels.Begin();
    Pixels.Show();
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
    }
}

void Patman::blank_leds()
{
    Anim.StopAll();
    Pixels.ClearTo(0);
    Pixels.Show();
}