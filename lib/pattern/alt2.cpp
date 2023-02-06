// Alt2 pattern by orangecalx 

#include "Alt2.h"




Alt2::Alt2(Patman *Pat, RgbColor color1, RgbColor color2)
{
    _color1 = color1;
    _color2 = color2;

    _Pat = Pat;
}

void Alt2::start()
{
    re_start();
}

void Alt2::re_start()
{
    _Pat->Anim.StartAnimation(0, 500, std::bind(&Alt2::update_function, this, std::placeholders::_1));
}                               // duration = (x) milliseconds, lower values are faster, etc

void Alt2::update_function(const AnimationParam &param)
{
       
    if (param.progress >= 0.5)  
    {
       
        _Pat->Pixels.ClearTo(_color1, 0, 0);
        _Pat->Pixels.ClearTo(RgbColor(0, 0, 0), 1, 1);
        _Pat->Pixels.ClearTo(_color1, 2, 3);
        _Pat->Pixels.ClearTo(RgbColor(0, 0, 0), 3, 3);
        _Pat->Pixels.ClearTo(_color1, 4, 4);
        _Pat->Pixels.ClearTo(RgbColor(0, 0, 0), 5, 5);


    }
    else
    {
        _Pat->Pixels.ClearTo(_color1, 5, 5);
        _Pat->Pixels.ClearTo(RgbColor(0, 0, 0), 4, 4);
        _Pat->Pixels.ClearTo(_color1, 3, 3);
        _Pat->Pixels.ClearTo(RgbColor(0, 0, 0), 2, 2);
        _Pat->Pixels.ClearTo(_color1, 1, 1);
        _Pat->Pixels.ClearTo(RgbColor(0, 0, 0), 0, 0);
    }

    if (param.state == AnimationState_Completed)
    {
        re_start();
    }
}
