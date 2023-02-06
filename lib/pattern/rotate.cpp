#include "Rotate.h"

Rotate::Rotate(Patman *Pat, RgbColor color)
{
    _color = color;
    _Pat = Pat;
}

void Rotate::start()
{
    _Pat->Pixels.ClearTo(0);
    _Pat->Anim.StartAnimation(0, 750, std::bind(&Rotate::update_function, this, std::placeholders::_1));
}




void Rotate::update_function(const AnimationParam &param)
{
    if (param.state == AnimationState_Progress)
    {
        
        _Pat->Pixels.ClearTo(0);
        
    }


   
    float progress = 1.0 - param.progress;                  // progress variable is 1.0 - the "clock"
    uint16_t n = progress * _Pat->Pixels.PixelCount();      // n variable is constrained to 0 - 5 range
    
    



    //_Pat->Pixels.SetPixelColor(n - 1, _color);
    _Pat->Pixels.SetPixelColor(n, _color);


    if (param.state == AnimationState_Completed)
    {
        start();
    }
}

