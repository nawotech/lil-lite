#include "battery_gauge.h"

BatteryGauge::BatteryGauge(Patman *Pat, Power *Pwr)
{
    _Pat = Pat;
    _Pwr = Pwr;
}

void BatteryGauge::start()
{
    _reverse = false;
    _Pat->Pixels.ClearTo(0);

    float batt_ratio = float(_Pwr->get_battery_percent()) / 100.0;
    uint16_t num_leds = _Pat->Pixels.PixelCount();
    _bat_leds = batt_ratio * num_leds;

    if (_bat_leds < 1) // always want at least 1 led on
    {
        _bat_leds = 1;
    }

    if (batt_ratio <= 0.1) // set color based on battery level
    {
        _color = RgbColor(255, 0, 0);
    }
    else if (batt_ratio <= 0.3)
    {
        _color = RgbColor(255, 128, 0);
    }
    else
    {
        _color = RgbColor(0, 100, 0);
    }

    _Pat->Anim.StartAnimation(0, 200, std::bind(&BatteryGauge::update_function, this, std::placeholders::_1));
}

void BatteryGauge::update_function(const AnimationParam &param)
{
    // this function fades _led_num LED from it's inital color to _color

    float progress = param.progress;
    if (_reverse)
    {
        progress = 1.0 - progress;
    }

    uint16_t led_n = progress * (_bat_leds - 1);

    _Pat->Pixels.SetPixelColor(led_n, _color);

    if (param.state == AnimationState_Completed & !_reverse)
    {
        _Pat->Anim.StartAnimation(0, 2000, std::bind(&BatteryGauge::update_function_2, this, std::placeholders::_1));
    }
}

void BatteryGauge::update_function_2(const AnimationParam &param)
{
    if (param.state == AnimationState_Completed)
    {
        _reverse = true;
        _color = 0;
        _Pat->Anim.StartAnimation(0, 300, std::bind(&BatteryGauge::update_function, this, std::placeholders::_1));
    }
}