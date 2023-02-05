// Grow animation random version, orangecalx

// same as default "Grow", but with random colors on each cycle


#include "growrand.h"

GrowRand::GrowRand(Patman *Pat, RgbColor color)
{
    
    _Pat = Pat;
    
}

void GrowRand::start()
{
    _Pat->Anim.StartAnimation(0, 700, std::bind(&GrowRand::update_function, this, std::placeholders::_1));
}

void GrowRand::update_function(const AnimationParam &param)
{
    if (param.state == AnimationState_Started)
    {
        _Pat->Pixels.ClearTo(0);
        _color = RgbColor(rand() % 150, rand() % 150, rand() % 150);
    }

    float progress = NeoEase::ExponentialOut(param.progress);
    // float progress = param.progress;

    // todo handle odd n of leds
    uint16_t z = _Pat->Pixels.PixelCount();
    uint16_t n = progress * z;
    
    //_Pat->Pixels.SetPixelColor(z - n - 1, _color);
    _Pat->Pixels.SetPixelColor(n, _color);

    if (param.state == AnimationState_Completed)
    {
        _Pat->Anim.StartAnimation(0, 10, std::bind(&GrowRand::update_function_2, this, std::placeholders::_1));
    }
}

void GrowRand::update_function_2(const AnimationParam &param)
{
    if (param.state == AnimationState_Started)
    {
        _Pat->Pixels.ClearTo(0);
    }

    if (param.state == AnimationState_Completed)
    {
        start();
        _color = RgbColor(0, rand() % 254 , rand() % 254);
    }
}