// alt pattern by orangecalx 

#include "alt.h"




Alt::Alt(Patman *Pat, RgbColor color1, RgbColor color2)
{
    _color1 = color1;
    _color2 = color2;

    _Pat = Pat;
}

void Alt::start()
{
    re_start();
}

void Alt::re_start()
{
    _Pat->Anim.StartAnimation(0, 460, std::bind(&Alt::update_function, this, std::placeholders::_1));
}                               // duration = (x) milliseconds, lower values are faster, etc

void Alt::update_function(const AnimationParam &param)
{
       
    if (param.progress >= 0.5)  
    {
       
        _Pat->Pixels.ClearTo(_color1, 0, 2);
        _Pat->Pixels.ClearTo(RgbColor(0, 0, 0), 3, 5);


    }
    else
    {
        _Pat->Pixels.ClearTo(_color2, 3, 5);
        _Pat->Pixels.ClearTo(RgbColor(0, 0, 0), 0, 2);
    }

    if (param.state == AnimationState_Completed)
    {
        re_start();
    }
}
