#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include "stdint.h"

class LightSensor
{
public:
    LightSensor(uint8_t analog_pin, uint8_t enable_pin, uint16_t night_mV_max = 2400);
    void begin();
    void enable();
    void disable();
    bool check_is_night();
    uint16_t read_mV();

private:
    uint8_t _analog_pin;
    uint8_t _enable_pin;
    uint16_t _night_mV_max;
};

#endif