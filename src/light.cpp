#include "light.h"
#include <Arduino.h>

Light::Light(LightSensor *LightSens, Patman *Pttrns, Motion *Mot, Button *Bttn, Power *Pwr, uint8_t sw_en_pin,
             Pattern *Pat_Power_On,
             Pattern *Pat_Power_Off,
             Pattern *Pat_Charging,
             Pattern *Pat_Charge_Done,
             Pattern *Pat_Battery_Level,
             Pattern *Pat_Moving,
             Pattern *Pat_Stopped)
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
    _Pat_Moving = Pat_Moving;
    _Pat_Stopped = Pat_Stopped;
}

void Light::begin(light_state_t inital_state)
{
    pinMode(_sw_en_pin, OUTPUT);
    digitalWrite(_sw_en_pin, 1);

    _LightSensor->begin();
    _Button->begin();
    _Power->begin();
    _Motion->begin();

    _Patterns->begin();

    _state = (light_state_t)(inital_state + 1); // force different state so it is update at first run
    set_state(inital_state);
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

        case OFF:
            _Patterns->blank_leds();
            break;
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
            set_state(ON);
        }
        break;

    case ON:
        if (power_state == CHARGING || power_state == USB_POWER)
        {
            set_state(CHARGE);
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
                _Patterns->set_pattern(_Pat_Moving);
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
        // todo write code
        break;
    }
}

light_state_t Light::get_state()
{
    return _state;
}