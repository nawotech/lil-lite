#include "flash.h"

Flash::Flash(Patman *Pat, RgbColor color)
{
    _color = color;
    _Pat = Pat;
}

void Flash::start()
{
    re_start();
}

void Flash::re_start()
{
    _Pat->Anim.StartAnimation(0, 300, std::bind(&Flash::update_function, this, std::placeholders::_1));
}

void Flash::update_function(const AnimationParam &param)
{
    // flash for 20% duty cycle
    if (param.progress >= 0.4 && param.progress <= 0.6)
    {
        _Pat->Pixels.ClearTo(_color);
    }
    else
    {
        _Pat->Pixels.ClearTo(0);
    }

    if (param.state == AnimationState_Completed)
    {
        re_start();
    }
}