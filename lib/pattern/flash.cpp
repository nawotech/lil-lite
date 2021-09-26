#include "flash.h"

Flash::Flash(Patman *Pat, RgbColor color)
{
    _color = color;
    _Pat = Pat;
}

void Flash::start()
{
    _count = 0;
    re_start();
}

void Flash::re_start()
{
    uint16_t speed;
    if (_count > 5)
    {
        _count = 0;
        speed = 750;
    }
    else
    {
        speed = 250;
    }
    _Pat->Anim.StartAnimation(0, speed, std::bind(&Flash::update_function, this, std::placeholders::_1));
    _count++;
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