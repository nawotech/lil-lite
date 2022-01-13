#include "power.h"

Power::Power(VoltageMonitor *vbat, VoltageMonitor *charge_current, uint8_t charge_stat_pin, uint8_t vbus_pin, float bat_capacity_mAh, float base_load_mA)
{
    _Vbat = vbat;
    _Icharge = charge_current;
    _charge_stat_pin = charge_stat_pin;
    _vbus_pin = vbus_pin;
    _bat_cap_mAh = bat_capacity_mAh;
    _base_load_mA = base_load_mA;
}

void Power::begin()
{
    pinMode(_charge_stat_pin, INPUT);
    pinMode(_vbus_pin, INPUT);
    _Vbat->begin();
    _Icharge->begin();
    // set_battery_load_current(0.0);
}

bool Power::is_usb_connected()
{
    return digitalRead(_vbus_pin);
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

    if (_bat_mAh <= 0.0)
    {
        _bat_mAh = 0;
    }

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
    power_state_t new_state = _state;

    bool usb_connected = is_usb_connected();
    bool charging = is_charging();
    bool low_battery = is_low_battery();

    switch (_state)
    {
    case BATTERY_POWER:
        if (usb_connected)
        {
            new_state = CHARGING;
        }
        else if (low_battery)
        {
            new_state = LOW_BATTERY;
        }
        break;

    case CHARGING:
        if (!usb_connected)
        {
            new_state = BATTERY_POWER;
        }
        if (!charging)
        {
            if (_charge_stopped)
            {
                if (_Tmr_VBUS.time_passed(100))
                {
                    new_state = USB_POWER;
                }
            }
            else
            {
                _charge_stopped = true;
                _Tmr_VBUS.reset();
            }
        }
        else if (_Tmr_load.get_ms() >= 30000) // update charge every 30 sec
        {
            update_battery_charge();
        }
        break;

    case USB_POWER:
        if (!usb_connected)
        {
            new_state = BATTERY_POWER;
        }
        if (charging)
        {
            new_state = CHARGING;
        }
        break;

    case LOW_BATTERY:
        if (usb_connected)
        {
            new_state = CHARGING;
        }
        break;
    }

    if (new_state != _state) // if state has changed
    {
        if (_state == CHARGING && new_state == USB_POWER) // when charge finishes
        {
            _bat_mAh = _bat_cap_mAh; // reset the charge to 100%
        }
        if (new_state == LOW_BATTERY) // if we hit low battery voltage
        {
            _bat_mAh = 0.0; // reset charge to 0%
        }
        if (_state == BATTERY_POWER) // leaving battery power state
        {
            update_battery_cap();
        }
        if (_state == CHARGING) // leaving charging state
        {
            _Tmr_load.reset();
            _charge_mA = 0.0;
        }
        if (_state == USB_POWER)
        {
            _Tmr_load.reset();
        }
        if (new_state == CHARGING) // entering charging state
        {
            _charge_stopped = false;
        }
        _state = new_state;
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
    if (_state == BATTERY_POWER)
    {
        update_battery_cap();
    }

    _Tmr_load.reset();
    _load_mA = _base_load_mA + mA;
}

float Power::get_battery_level_mAh()
{
    return _bat_mAh;
}
void Power::set_battery_level_mAh(float mAh)
{
    _bat_mAh = mAh;
}