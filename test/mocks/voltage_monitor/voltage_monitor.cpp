#include "voltage_monitor.h"

uint16_t VoltageMonitor::get_mV()
{
    return _MOCK_mV;
}

void VoltageMonitor::MOCK_get_mV(uint16_t mV)
{
    _MOCK_mV = mV;
}

void VoltageMonitor::begin()
{
    return;
}