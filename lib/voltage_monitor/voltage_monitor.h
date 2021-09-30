#ifndef VOLTAGE_MONITOR_H
#define VOLTAGE_MONITOR_H

#include <stdint.h>

class VoltageMonitor
{
public:
    VoltageMonitor(uint8_t pin, float scale);
    void begin();
    uint16_t get_mV();

private:
    float _scale;
    uint8_t _pin;
};

#endif