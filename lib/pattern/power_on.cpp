#include "power_on.h"

PowerOn::PowerOn(Patman *Pat)
{
    _color = RgbColor(50, 30, 30);
    _Pat = Pat;
}

void PowerOn::start()
{
    _Pat->Anim.StartAnimation(0, 500, std::bind(&PowerOn::update_function, this, std::placeholders::_1));
}

void PowerOn::update_function(const AnimationParam &param)
{
    if (param.state == AnimationState_Started)
    {
        _Pat->Pixels.ClearTo(0);
    }

    float progress = NeoEase::ExponentialOut(param.progress);

    // animate LEDs out from center
    // todo handle odd n of leds
    // leds to light
    uint16_t z = _Pat->Pixels.PixelCount() / 2;
    uint16_t n = progress * z;

    _Pat->Pixels.SetPixelColor(z - n - 1, _color);
    _Pat->Pixels.SetPixelColor(z + n, _color);
}