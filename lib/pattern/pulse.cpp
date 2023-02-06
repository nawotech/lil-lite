#include "pulse.h"

Pulse::Pulse(Patman *Pat, RgbColor color)
{
    _color = color;
    _Pat = Pat;
    _speed = 2000;
}

void Pulse::start()
{
    _Pat->Anim.StartAnimation(0, _speed, std::bind(&Pulse::update_function, this, std::placeholders::_1));
}

void Pulse::update_function(const AnimationParam &param)
{
    float progress = NeoEase::Linear(param.progress);
    RgbColor this_color = _color;

    if (progress <= 0.5) {
        this_color.Darken(512 * progress);

    } 

    if (progress > 0.5) {
        this_color.Darken(-512 * progress);
    }

    
    

    _Pat->Pixels.ClearTo(this_color);

    if (param.state == AnimationState_Completed)
    {
        start();
    }
}