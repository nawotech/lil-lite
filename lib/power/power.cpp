#include "power.h"

Power::Power(VoltageMonitor *vbus, VoltageMonitor *vbat, VoltageMonitor *charge_current, uint8_t charge_stat_pin, float bat_capacity_mAh)
{
    _Vbus = vbus;
    _Vbat = vbat;
    _Icharge = charge_current;
    _charge_stat_pin = charge_stat_pin;
    _bat_cap_mAh = bat_capacity_mAh;
}

void Power::begin()
{
    pinMode(_charge_stat_pin, INPUT);
    _Vbus->begin();
    _Vbat->begin();
    _Icharge->begin();
}

bool Power::is_usb_connected()
{
    uint16_t mv_vbus = _Vbus->get_mV();
    if (mv_vbus > 4000)
    {
        return true;
    }
    return false;
}

bool Power::is_charging()
{
    if (digitalRead(_charge_stat_pin) == 0)
    {
        return true;
    }
    return false;
}

void Power::update_battery_cap()
{
    // calculate previous load mAh
    float last_mAh = _load_mA * (float)_Tmr_load.get_ms() / 1000.0 / 60.0 / 60.0;

    // subtract from current charge
    _bat_mAh = _bat_mAh - last_mAh;

    // reset the timer to start next segment of time to measure
    _Tmr_load.reset();
}

void Power::update_battery_charge()
{
    // calculate previous load mAh
    float last_mAh = _charge_mA * (float)_Tmr_load.get_ms() / 1000.0 / 60.0 / 60.0;

    // subtract from current charge
    _bat_mAh = _bat_mAh + last_mAh;

    if (_bat_mAh > _bat_cap_mAh)
    {
        _bat_mAh = _bat_cap_mAh;
    }

    // reset the timer to start next segment of time to measure
    _Tmr_load.reset();

    _charge_mA = (float)_Icharge->get_mV() / 10.0; // read charge current and update
}

// low battery cutoff, 3.3V for lithium
bool Power::is_low_battery()
{
    uint16_t mv_vbat = _Vbat->get_mV();
    if (mv_vbat < 3300)
    {
        return true;
    }
    return false;
}

void Power::update()
{
    power_state_t new_state;

    if (is_usb_connected())
    {
        if (is_charging())
        {
            new_state = CHARGING;
        }
        else
        {
            new_state = USB_POWER;
        }
    }
    else
    {
        if (is_low_battery())
        {
            new_state = LOW_BATTERY;
        }
        else
        {
            new_state = BATTERY_POWER;
        }
    }

    if (new_state != _state) // if state has changed
    {
        if (_state == CHARGING && new_state == USB_POWER) // when charge finishes
        {
            _bat_mAh = _bat_cap_mAh; // reset the charge to 100%
        }
        else if (_state == BATTERY_POWER && new_state == LOW_BATTERY) // if we hit low battery voltage
        {
            _bat_mAh = 0.0; // reset charge to 0%
        }
        else if (_state == BATTERY_POWER) // leaving battery power state
        {
            update_battery_cap();
        }
        else if (_state == CHARGING)
        {
            _Tmr_load.reset();
            _charge_mA = 0.0;
        }
    }

    _state = new_state;

    if (_state == CHARGING)
    {
        if (_Tmr_load.get_ms() >= 30000) // update charge every 30 sec
        {
            update_battery_charge();
        }
    }
}

power_state_t Power::get_state()
{
    return _state;
}

uint8_t Power::get_battery_percent()
{
    if (_state == BATTERY_POWER)
    {
        update_battery_cap();
    }
    return _bat_mAh / _bat_cap_mAh * 100.0;
}

void Power::set_battery_load_current(float mA)
{
    if (mA != _load_mA)
    {
        if (_state == BATTERY_POWER)
        {
            update_battery_cap();
        }

        _Tmr_load.reset();
        _load_mA = mA;
    }
}

float Power::get_battery_level_mAh()
{
    return _bat_mAh;
}
void Power::set_battery_level_mAh(float mAh)
{
    _bat_mAh = mAh;
}