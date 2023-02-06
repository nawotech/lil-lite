#include "growrev.h"

GrowRev::GrowRev(Patman *Pat, RgbColor color)
{
    _color = color;
    _Pat = Pat;
}

void GrowRev::start()
{
    _Pat->Anim.StartAnimation(0, 1500, std::bind(&GrowRev::update_function, this, std::placeholders::_1));
}

void GrowRev::update_function(const AnimationParam &param)
{
    if (param.state == AnimationState_Started)
    {
        _Pat->Pixels.ClearTo(0);
    }

    // float progress = NeoEase::ExponentialOut(param.progress);
    float progress = 1.0 - param.progress;

    // todo handle odd n of leds
    uint16_t z = _Pat->Pixels.PixelCount() / 2;
    uint16_t n = progress * z;

    _Pat->Pixels.SetPixelColor(z - n - 1, _color);
    _Pat->Pixels.SetPixelColor(z + n, _color);

    
    if (param.state == AnimationState_Completed)
    {
        _Pat->Anim.StartAnimation(0, 300, std::bind(&GrowRev::update_function_2, this, std::placeholders::_1));
    }
}


void GrowRev::update_function_2(const AnimationParam &param)
{
    if (param.state == AnimationState_Started)
    {
        _Pat->Pixels.ClearTo(0);
    }

    if (param.state == AnimationState_Completed)
    {
        start();
    }
}
