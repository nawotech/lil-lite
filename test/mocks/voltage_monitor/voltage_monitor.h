#ifndef VOLTAGE_MONITOR_H
#define VOLTAGE_MONITOR_H

#include <stdint.h>

class VoltageMonitor
{
public:
    uint16_t get_mV();
    void begin();

    void MOCK_get_mV(uint16_t mV);

private:
    float _scale;
    uint8_t _pin;
    uint16_t _MOCK_mV;
};

#endif