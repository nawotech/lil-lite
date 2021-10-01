#ifndef LIGHT_H
#define LIGHT_H

#include "light_sensor.h"
#include "patman.h"
#include "motion.h"
#include "button.h"
#include "power.h"

typedef enum
{
    POWERING_ON,
    BATTERY_GAUGE,
    ON,
    CHARGE,
    POWERING_OFF,
    OFF,
    PARKED,
    DAY_RIDING
} light_state_t;

class Light
{
public:
    Light(LightSensor *LightSens, Patman *Pttrns, Motion *Mot, Button *Bttn, Power *Pwr, uint8_t sw_en_pin,
          Pattern *Pat_Power_On,
          Pattern *Pat_Power_Off,
          Pattern *Pat_Charging,
          Pattern *Pat_Charge_Done,
          Pattern *Pat_Battery_Level,
          Pattern *Pat_Moving,
          Pattern *Pat_Stopped);
    void begin(light_state_t inital_state = POWERING_ON);
    void update();
    light_state_t get_state();

private:
    void set_state(light_state_t new_state);

    LightSensor *_LightSensor;
    Patman *_Patterns;
    Motion *_Motion;
    Button *_Button;
    Power *_Power;
    light_state_t _state;
    uint8_t _sw_en_pin;

    Pattern *_Pat_Power_On;
    Pattern *_Pat_Power_Off;
    Pattern *_Pat_Charging;
    Pattern *_Pat_Charge_Done;
    Pattern *_Pat_Battery_Level;
    Pattern *_Pat_Moving;
    Pattern *_Pat_Stopped;
};

#endif