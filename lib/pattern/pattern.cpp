#include "pattern.h"

Pattern::Pattern(NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> *Leds) : _Anim(1)
{
    _Pixels = Leds;
}

void Pattern::begin()
{
    _Pixels->Begin();
    _Pixels->Show();
    _num_leds = _Pixels->PixelCount();
    _color[0] = RgbColor(255, 0, 0);
    _color[1] = RgbColor(255, 255, 0);
    _color[2] = RgbColor(0, 255, 50);
    _charge = RgbColor(0, 100, 0);
}

void Pattern::update()
{
    if (_Anim.IsAnimating())
    {
        _Anim.UpdateAnimations();
        _Pixels->Show();
    }
    else
    {
        switch (_pattern)
        {
        case PATTERN_MOVE:
            _color_n++;
            if (_color_n > 2)
            {
                _color_n = 0;
            }
            _Anim.StartAnimation(0, 400, std::bind(&Pattern::pattern_move, this, std::placeholders::_1));
            break;
        case PATTERN_STOPPED:
            _color_n++;
            if (_color_n > 2)
            {
                _color_n = 0;
            }
            _Anim.StartAnimation(0, 1000, std::bind(&Pattern::pattern_stop, this, std::placeholders::_1));
            break;
        case PATTERN_CHARGE:
            _Anim.StartAnimation(0, 3000, std::bind(&Pattern::pattern_charge, this, std::placeholders::_1));
            break;
        }
    }
}

void Pattern::start(pattern_t pattern)
{
    if (pattern != _pattern)
    {
        _Anim.StopAll();
        _pattern = pattern;
    }
}

bool Pattern::is_running()
{
    return _Anim.IsAnimating();
}

void Pattern::pattern_move(const AnimationParam &param)
{
    float progress = NeoEase::CircularInOut(param.progress);

    uint8_t led_n = progress * (_num_leds + 1);

    _Pixels->SetPixelColor(led_n, _color[_color_n]);
    for (int i = 0; i < _num_leds; i++)
    {
        if (i != led_n)
        {
            _Pixels->SetPixelColor(i, 0);
        }
    }
}

void Pattern::pattern_stop(const AnimationParam &param)
{
    float progress = NeoEase::CircularInOut(param.progress);

    RgbColor color = _color[_color_n];
    color.Darken(progress * 255);

    _Pixels->ClearTo(color);
}

void Pattern::pattern_charge(const AnimationParam &param)
{
    float progress = param.progress;

    uint16_t led_n;

    led_n = progress * _battery_percent * 6 / 100;

    _Pixels->SetPixelColor(led_n, _color[_color_n]);
    for (int i = 0; i < _num_leds; i++)
    {
        if (i <= led_n)
        {
            _Pixels->SetPixelColor(i, _charge);
        }
        else
        {
            _Pixels->SetPixelColor(i, 0);
        }
    }
}

void Pattern::set_battery_gauge_percent(uint8_t percent)
{
    _battery_percent = percent;
}