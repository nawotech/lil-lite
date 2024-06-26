#include "voltage_monitor.h"
#include "analog.h"
#include <Arduino.h>

VoltageMonitor::VoltageMonitor(uint8_t pin, float scale)
{
    _pin = pin;
    _scale = scale;
}

uint16_t VoltageMonitor::get_mV()
{
    return analog_get_mV(_pin, 20) * _scale;
}

void VoltageMonitor::begin()
{
    pinMode(_pin, INPUT);
}