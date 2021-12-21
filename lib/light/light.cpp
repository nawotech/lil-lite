#include "light.h"
#include <Arduino.h>

Light::Light(LightSensor *LightSens, Patman *Pttrns, Motion *Mot, Button *Bttn, Power *Pwr, uint8_t sw_en_pin,
             Pattern *Pat_Power_On,
             Pattern *Pat_Power_Off,
             Pattern *Pat_Charging,
             Pattern *Pat_Charge_Done,
             Pattern *Pat_Battery_Level,
             Pattern *Pat_Stopped,
             Pattern *Pat_Wifi_Control,
             Pattern **Pats_Moving,
             uint16_t num_pats_moving)
{
    _LightSensor = LightSens;
    _Patterns = Pttrns;
    _Motion = Mot;
    _Button = Bttn;
    _Power = Pwr;

    _sw_en_pin = sw_en_pin;

    _Pat_Power_On = Pat_Power_On;
    _Pat_Power_Off = Pat_Power_Off;
    _Pat_Charging = Pat_Charging;
    _Pat_Charge_Done = Pat_Charge_Done;
    _Pat_Battery_Level = Pat_Battery_Level;
    _Pats_Moving = Pats_Moving;
    _Pat_Stopped = Pat_Stopped;
    _Pat_Wifi_Control = Pat_Wifi_Control;
    _num_pats_moving = num_pats_moving;

    _current_pat_moving = 1;
}

void Light::begin(light_state_t inital_state)
{
    _LightSensor->begin();
    _Button->begin();
    _Power->begin();
    _Motion->begin();

    if (inital_state == PARKED)
    {
        if (!_Button->is_pressed() && !_Power->is_usb_connected())
        {
            delay(100);
            if (!_LightSensor->check_is_night())
            {
                // day time
                _state = DAY_RIDING;
                return;
            }
        }
    }
    else if (inital_state == DAY_RIDING)
    {
        if (!_Button->is_pressed() && !_Power->is_usb_connected())
        {
            _state = PARKED;
            return;
        }
    }

    pinMode(_sw_en_pin, OUTPUT);
    digitalWrite(_sw_en_pin, 1);
    delay(100);
    _Patterns->begin();

    _state = inital_state; // force different state so it is update at first run
    set_state(POWERING_ON);
}

void Light::set_state(light_state_t new_state)
{
    if (new_state != _state)
    {
        switch (new_state)
        {
        case POWERING_ON:
            _Patterns->set_pattern(_Pat_Power_On);
            break;

        case BATTERY_GAUGE:
            _Patterns->set_pattern(_Pat_Battery_Level);
            break;

        case POWERING_OFF:
            _Patterns->set_pattern(_Pat_Power_Off);
            break;

        case POWERING_OFF_FROM_WIFI:
            _Patterns->set_pattern(_Pat_Power_Off);
            break;

        case OFF:
            _Patterns->blank_leds();
            break;

        case PARKED:
            _Patterns->blank_leds();
            break;

        case SELECTING_PATTERN:
            _LightTmr.reset();
            break;

        case WIFI_CONTROL:
            _Patterns->set_pattern(_Pat_Wifi_Control);
        }

        if (new_state == BATTERY_GAUGE)
        {
            _Button->set_multi_press_enabled(true);
        }
        else
        {
            _Button->set_multi_press_enabled(false);
        }

        _state = new_state;
    }
}

void Light::update()
{
    _Patterns->update();
    _Motion->update();
    _Button->update();
    _Power->update();

    power_state_t power_state = _Power->get_state();
    motion_state_t motion_state = _Motion->get_state();
    button_state_t button_state = _Button->get_state();

    // handle state
    switch (_state)
    {
    case POWERING_ON:
        if (!_Patterns->is_running())
        {
            set_state(BATTERY_GAUGE);
        }
        break;

    case BATTERY_GAUGE:
        if (power_state == CHARGING || power_state == USB_POWER)
        {
            set_state(CHARGE);
        }
        else if (!_Patterns->is_running())
        {
            if (power_state == LOW_BATTERY)
            {
                set_state(OFF);
            }
            else
            {
                set_state(ON);
            }
        }
        else if (button_state == BUTTON_TRIPLE_PRESS)
        {
            set_state(WIFI_CONTROL);
        }
        else if (button_state == BUTTON_SHORT_PRESS)
        {
            set_state(SELECTING_PATTERN);
        }
        break;

    case ON:
        if (power_state == CHARGING || power_state == USB_POWER)
        {
            set_state(CHARGE);
        }
        else if (power_state == LOW_BATTERY)
        {
            set_state(BATTERY_GAUGE);
        }
        else if (button_state == BUTTON_SHORT_PRESS)
        {
            set_state(BATTERY_GAUGE);
        }
        else if (button_state == BUTTON_LONG_HOLD_START)
        {
            set_state(POWERING_OFF);
        }
        else
        {
            switch (motion_state)
            {
            case MOTION_MOVING:
                _Patterns->set_pattern(_Pats_Moving[_current_pat_moving]);
                break;

            case MOTION_STOPPED:
                _Patterns->set_pattern(_Pat_Stopped);
                break;

            case MOTION_PARKED:
                set_state(PARKED);
                break;
            }
        }
        break;

    case POWERING_OFF:
        if (!_Patterns->is_running())
        {
            set_state(OFF);
        }
        else if (!_Button->is_pressed())
        {
            set_state(ON);
        }
        break;

    case POWERING_OFF_FROM_WIFI:
        if (!_Patterns->is_running())
        {
            set_state(OFF);
        }
        else if (!_Button->is_pressed())
        {
            set_state(WIFI_CONTROL);
        }
        break;

    case CHARGE:
        if (power_state == USB_POWER)
        {
            _Patterns->set_pattern(_Pat_Charge_Done);
        }
        else if (power_state == CHARGING)
        {
            _Patterns->set_pattern(_Pat_Charging);
        }
        else
        {
            set_state(OFF);
        }

    case PARKED:
        break;

    case OFF:
        break;

    case DAY_RIDING:
        break;

    case SELECTING_PATTERN:
        _Patterns->set_pattern(_Pats_Moving[_current_pat_moving]);
        if (_LightTmr.time_passed(10000))
        {
            set_state(ON);
        }
        else if (button_state == BUTTON_SHORT_PRESS)
        {
            _current_pat_moving++;
            if (_current_pat_moving >= _num_pats_moving)
            {
                _current_pat_moving = 0;
            }
        }
        break;

    case WIFI_CONTROL:
        if (power_state == LOW_BATTERY)
        {
            set_state(BATTERY_GAUGE);
        }
        else if (button_state == BUTTON_LONG_HOLD_START)
        {
            set_state(POWERING_OFF_FROM_WIFI);
        }
    }
}

light_state_t Light::get_state()
{
    return _state;
}

uint16_t Light::get_moving_pattern_num()
{
    return _current_pat_moving;
}

void Light::set_moving_pattern_num(uint16_t num)
{
    if (num >= _num_pats_moving)
    {
        _current_pat_moving = 0;
    }
    else
    {
        _current_pat_moving = num;
    }
}