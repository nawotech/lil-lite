#include "jump.h"

Jump::Jump(Patman *Pat, RgbColor color)
{
    _color = color;
    _Pat = Pat;
}

void Jump::start()
{
    _Pat->Pixels.ClearTo(0);
    _Pat->Anim.StartAnimation(0, 500, std::bind(&Jump::update_function, this, std::placeholders::_1));
}

void Jump::update_function(const AnimationParam &param)
{
    if (param.state == AnimationState_Progress)
    {
        _Pat->Pixels.ClearTo(0);
    }

    float progress = NeoEase::CircularOut(param.progress);
    // float progress = param.progress;

    // todo handle odd n of leds
    uint16_t n = progress * _Pat->Pixels.PixelCount();

    _Pat->Pixels.SetPixelColor(n, _color);

    if (param.state == AnimationState_Completed)
    {
        _Pat->Anim.StartAnimation(0, 400, std::bind(&Jump::update_function_2, this, std::placeholders::_1));
    }
}

void Jump::update_function_2(const AnimationParam &param)
{
    if (param.state == AnimationState_Progress)
    {
        _Pat->Pixels.ClearTo(0);
    }

    float progress = 1.0 - NeoEase::CircularIn(param.progress);
    // float progress = param.progress;

    // todo handle odd n of leds
    uint16_t n = progress * _Pat->Pixels.PixelCount();

    _Pat->Pixels.SetPixelColor(n, _color);

    if (param.state == AnimationState_Completed)
    {
        _Pat->Anim.StartAnimation(0, 400, std::bind(&Jump::update_function, this, std::placeholders::_1));
    }
}