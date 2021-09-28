#include "charge.h"

Charge::Charge(Patman *Pat, Power *Pwr)
{
    _Pat = Pat;
    _Pwr = Pwr;
}

void Charge::start()
{
    _Pat->Pixels.ClearTo(0);
    RgbColor ChargeColorGreen(0, 20, 0);
    _color = ChargeColorGreen;
    _led_num = 0;
    _bat_leds = _Pat->Pixels.PixelCount();
    re_start();
}

void Charge::re_start()
{
    if (_led_num > _bat_leds - 1)
    {
        float batt_ratio = float(_Pwr->get_battery_percent()) / 100.0;
        uint16_t num_leds = _Pat->Pixels.PixelCount();
        _bat_leds = batt_ratio * num_leds;
        if (_bat_leds < 2) // always want at least 2 leds on
        {
            _bat_leds = 2;
        }

        _led_num = 0;
        _Pat->Pixels.ClearTo(0);
        RgbColor dim(0, 2, 0);
        for (int i = 0; i < _bat_leds; i++)
        {
            _Pat->Pixels.SetPixelColor(i, dim);
        }
    }
    _Pat->Anim.StartAnimation(0, 1000, std::bind(&Charge::update_function, this, std::placeholders::_1));
}

void Charge::update_function(const AnimationParam &param)
{
    // this function fades _led_num LED from it's inital color to _color

    if (param.state == AnimationState_Started)
    {
        _prev_pixel_color = _Pat->Pixels.GetPixelColor(_led_num);
    }

    float progress = param.progress;

    RgbColor new_color = RgbColor::LinearBlend(_prev_pixel_color, _color, progress);
    _Pat->Pixels.SetPixelColor(_led_num, new_color);

    if (param.state == AnimationState_Completed)
    {
        _led_num++;
        re_start();
    }
}