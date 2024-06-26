#ifndef POWER_H
#define POWER_H

#include <Arduino.h>

#include "voltage_monitor.h"
#include "timer.h"

typedef enum
{
    CHARGING,
    BATTERY_POWER,
    USB_POWER,
    LOW_BATTERY
} power_state_t;

class Power
{
public:
    Power(VoltageMonitor *vbat, VoltageMonitor *charge_current, uint8_t charge_stat_pin, uint8_t vbus_pin, float bat_capacity_mAh, float base_load_mA);
    void update();
    void begin();
    power_state_t get_state();
    uint8_t get_battery_percent();
    void set_battery_load_current(float mA);
    bool is_usb_connected();

    float get_battery_level_mAh();
    void set_battery_level_mAh(float mAh);

    Timer _Tmr_load;
    Timer _Tmr_VBUS; // keep public for unit testing mocks

private:
    bool is_charging();
    bool is_low_battery();
    void update_battery_cap();
    void update_battery_charge();

    VoltageMonitor *_Vbat;
    VoltageMonitor *_Icharge;
    uint8_t _charge_stat_pin;
    uint8_t _vbus_pin;

    power_state_t _state = BATTERY_POWER;

    float _bat_cap_mAh;
    float _charge_mA = 0.0;
    float _bat_mAh;
    float _load_mA = 0.0;
    float _base_load_mA;
    bool _charge_stopped = false;
};

#endif