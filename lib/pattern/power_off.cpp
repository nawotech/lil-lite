#include "power_off.h"

PowerOff::PowerOff(Patman *Pat)
{
    _color = RgbColor(255, 0, 0);
    _Pat = Pat;
}

void PowerOff::start()
{
    _Pat->Anim.StartAnimation(0, 3000, std::bind(&PowerOff::update_function, this, std::placeholders::_1));
}

void PowerOff::update_function(const AnimationParam &param)
{
    if (param.state == AnimationState_Started)
    {
        _Pat->Pixels.ClearTo(0);
    }

    // float progress = NeoEase::ExponentialOut(param.progress);
    float progress = 1.0 - param.progress;

    // animate LEDs out from center
    // todo handle odd n of leds
    // leds to light
    uint16_t z = _Pat->Pixels.PixelCount() / 2;
    uint16_t n = progress * z;

    _Pat->Pixels.SetPixelColor(z - n - 1, _color);
    _Pat->Pixels.SetPixelColor(z + n, _color);
}