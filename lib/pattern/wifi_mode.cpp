#include "wifi_mode.h"

WifiMode::WifiMode(Patman *Pat)
{
    _color = RgbColor(30, 0, 50);
    _Pat = Pat;
    _speed = 1000;
}

void WifiMode::start()
{
    _Pat->Pixels.ClearTo(0);
    _Pat->Anim.StartAnimation(0, _speed, std::bind(&WifiMode::update_function, this, std::placeholders::_1));
}

void WifiMode::update_function(const AnimationParam &param)
{
    float progress = NeoEase::QuadraticInOut(param.progress);

    RgbColor this_color = _color;
    this_color.Darken(255 * progress);

    _Pat->Pixels.SetPixelColor(2, this_color);
    _Pat->Pixels.SetPixelColor(3, this_color);

    if (param.state == AnimationState_Completed)
    {
        start();
    }
}