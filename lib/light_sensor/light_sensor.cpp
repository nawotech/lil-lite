#include "light_sensor.h"
#include <Arduino.h>
#include "analog.h"

LightSensor::LightSensor(uint8_t analog_pin, uint8_t enable_pin, uint16_t night_mV_max)
{
    _analog_pin = analog_pin;
    _enable_pin = enable_pin;
    _night_mV_max = night_mV_max;
}

void LightSensor::begin()
{
    pinMode(_analog_pin, INPUT);
    pinMode(_enable_pin, OUTPUT);
    enable();
}

void LightSensor::enable()
{
    digitalWrite(_enable_pin, 1);
}

void LightSensor::disable()
{
    digitalWrite(_enable_pin, 0);
}

uint16_t LightSensor::read_mV()
{
    return analog_get_mV(_analog_pin, 10);
}

bool LightSensor::check_is_night()
{
    uint16_t mV = read_mV();
    if (mV <= _night_mV_max)
    {
        return true;
    }
    return false;
}