#include "fade.h"

Fade::Fade(Patman *Pat, RgbColor color)
{
    _color = color;
    _Pat = Pat;
    _speed = 3000;
}

void Fade::start()
{
    _Pat->Anim.StartAnimation(0, _speed, std::bind(&Fade::update_function, this, std::placeholders::_1));
}

void Fade::update_function(const AnimationParam &param)
{
    float progress = NeoEase::QuadraticInOut(param.progress);

    RgbColor this_color = _color;
    this_color.Darken(255 * progress);

    _Pat->Pixels.ClearTo(this_color);

    if (param.state == AnimationState_Completed)
    {
        start();
    }
}